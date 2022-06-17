#include "Renderer.h"
#include "Scene.h"
#include "Object.h"
#include "Material.h"

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <array>
#include <cstddef>

static std::vector<char> readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void TEForwardRenderer::Init(TEPtr<TEDevice> device, TEPtr<TEWindow> window)
{
    _device = device;
    _window = window;

    SelectSurfaceFormat();

    _vkRenderPass = _device->CreateRenderPass(_vkSurfaceFormat.format);
    CreateSwapchain(_vkRenderPass);

    _vkCommandPool = _device->CreateCommandPool(_device->graphicQueueFamilyIndex);
    _vkCommandBuffer = _device->CreateCommandBuffer(_vkCommandPool);

    _imageAvailableSemaphore = _device->CreateSemaphore();
    _renderFinishedSemaphore = _device->CreateSemaphore();

    _inFlightFence = _device->CreateFence(true);
}

void TEForwardRenderer::SelectSurfaceFormat()
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_device->vkPhysicalDevice, _window->vkSurface, &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    if (formatCount != 0)
    {
        surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_device->vkPhysicalDevice, _window->vkSurface, &formatCount, surfaceFormats.data());
    }

    for (size_t i = 0; i < surfaceFormats.size(); i++)
    {
        const VkSurfaceFormatKHR &currentSurfaceFormat = surfaceFormats[i];

        if (currentSurfaceFormat.colorSpace == VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && currentSurfaceFormat.format == VkFormat::VK_FORMAT_B8G8R8A8_SRGB)
        {
            _vkSurfaceFormat = currentSurfaceFormat;
            break;
        }
    }
}


VkPipeline TEForwardRenderer::CreatePipeline(TEPtr<TEMaterial> material)
{
    auto vertShaderCode = readFile("Shaders/VertexShader.spv");
    auto fragShaderCode = readFile("Shaders/FragmentShader.spv");

    // Shader
    VkShaderModule vkVerterShaderModule = _device->CreateShaderModule(vertShaderCode);
    _vkShaderModules.push_back(vkVerterShaderModule);
    VkShaderModule vkFragmentShaderModule = _device->CreateShaderModule(fragShaderCode);
    _vkShaderModules.push_back(vkFragmentShaderModule);

    _vkPipelineLayout = _device->CreatePipelineLayout();
    VkPipeline vkPipeline = _device->CreateGraphicPipeline(vkVerterShaderModule, vkFragmentShaderModule, _vkExtent, _vkPipelineLayout, _vkRenderPass);

    return vkPipeline;
}

void TEForwardRenderer::CreateSwapchain(VkRenderPass renderPass)
{
    // Swap Chain
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_device->vkPhysicalDevice, _window->vkSurface, &presentModeCount, nullptr);

    std::vector<VkPresentModeKHR> presentModes;
    if (presentModeCount != 0)
    {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_device->vkPhysicalDevice, _window->vkSurface, &presentModeCount, presentModes.data());
    }

    VkPresentModeKHR presentMode{};
    for (size_t i = 0; i < presentModes.size(); i++)
    {
        const VkPresentModeKHR &currentPresentMode = presentModes[i];

        if (currentPresentMode == VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = currentPresentMode;
            break;
        }
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->vkPhysicalDevice, _window->vkSurface, &capabilities);

    // GLFW uses two units when measuring sizes: pixels and screen coordinates.

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        _vkExtent = capabilities.currentExtent;
    }
    else
    {

        // glfwGetFramebufferSize: resolution of the window in pixel
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(_window->glfwWindow, &framebufferWidth, &framebufferHeight);

        _vkExtent.width = std::clamp(static_cast<uint32_t>(framebufferWidth), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        _vkExtent.height = std::clamp(static_cast<uint32_t>(framebufferHeight), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    _vkSwapchain = _device->CreateSwapchain(capabilities.minImageCount + 1, _vkSurfaceFormat.format, _vkSurfaceFormat.colorSpace, _vkExtent, capabilities.currentTransform, presentMode);

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(_device->vkDevice, _vkSwapchain, &imageCount, nullptr);
    _vkImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device->vkDevice, _vkSwapchain, &imageCount, _vkImages.data());

    // Image View
    _vkImageViews.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        _vkImageViews[i] = _device->CreateImageView(_vkImages[i], _vkSurfaceFormat.format);
    }

    _vkFramebuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        std::vector<VkImageView> attachments = {
            _vkImageViews[i]};

        _vkFramebuffers[i] = _device->CreateFramebuffer(renderPass, attachments, _vkExtent.width, _vkExtent.height);
    }
}

void TEForwardRenderer::GatherObjects(TEPtr<TEScene> scene)
{
    const TEPtrArr<TEObject> &objects = scene->GetObjects();

    for (auto &object : objects)
    {
        TEPtr<TEMaterial> material = object->_material;
        std::uintptr_t address = reinterpret_cast<std::uintptr_t>(material.get());
        if (_objectsToRender.find(address) == _objectsToRender.end())
            _objectsToRender.emplace(address, TEPtrArr<TEObject>());
        TEPtrArr<TEObject> &objectArr = _objectsToRender.at(address);
        objectArr.push_back(object);
    }
}

void TEForwardRenderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _vkCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device->vkDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(_device->vkGraphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_device->vkGraphicQueue);

    vkFreeCommandBuffers(_device->vkDevice, _vkCommandPool, 1, &commandBuffer);
}

void TEForwardRenderer::RenderFrame(TEPtr<TEScene> scene)
{
    GatherObjects(scene);

    vkWaitForFences(_device->vkDevice, 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(_device->vkDevice, 1, &_inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(_device->vkDevice, _vkSwapchain, UINT64_MAX, _imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _vkRenderPass;
    renderPassInfo.framebuffer = _vkFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _vkExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    if (vkBeginCommandBuffer(_vkCommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    vkCmdBeginRenderPass(_vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (auto &pair : _objectsToRender)
    {
        auto &objectArr = pair.second;
        if (objectArr.empty())
            continue;

        TEPtr<TEMaterial> material = objectArr[0]->_material;

        VkPipeline vkPipeline;

        std::uintptr_t address = reinterpret_cast<std::uintptr_t>(material.get());
        if (_pipelines.find(address) == _pipelines.end())
        {
            vkPipeline = CreatePipeline(material);
            _pipelines.insert(std::make_pair(address, vkPipeline));
        }
        else
        {
            vkPipeline = _pipelines[address];
        }

        size_t totalBufferSize = 0;
        for (auto &object : objectArr)
        {
            totalBufferSize = object->vertices.size() * sizeof(glm::vec3);
        }

        if (_stagingBuffer != VK_NULL_HANDLE && totalBufferSize > _stagingBufferSize)
        {
            vkFreeMemory(_device->vkDevice, _stagingBufferMemory, nullptr);
            vkDestroyBuffer(_device->vkDevice, _stagingBuffer, nullptr);
            _stagingBuffer = VK_NULL_HANDLE;
        }

        if (_stagingBuffer == VK_NULL_HANDLE)
        {
            _stagingBuffer = _device->CreateBuffer(totalBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            _stagingBufferMemory = _device->AllocateAndBindBufferMemory(_stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }

        if (_vertexBuffer != VK_NULL_HANDLE && totalBufferSize > _vertexBufferSize)
        {
            vkFreeMemory(_device->vkDevice, _vertexBufferMemory, nullptr);
            vkDestroyBuffer(_device->vkDevice, _vertexBuffer, nullptr);
            _vertexBuffer = VK_NULL_HANDLE;
        }

        if (_vertexBuffer == VK_NULL_HANDLE)
        {
            _vertexBuffer = _device->CreateBuffer(totalBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            _vertexBufferMemory = _device->AllocateAndBindBufferMemory(_vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }

        void *data;
        vkMapMemory(_device->vkDevice, _stagingBufferMemory, 0, totalBufferSize, 0, &data);

        uint8_t *dataPtr = reinterpret_cast<uint8_t *>(data);
        uint32_t vertexCounts = 0;
        for (auto &object : objectArr)
        {
            size_t bufferSize = object->vertices.size() * sizeof(glm::vec3);
            memcpy(dataPtr, object->vertices.data(), bufferSize);
            dataPtr = dataPtr + bufferSize;
            vertexCounts += object->vertices.size();
        }

        vkUnmapMemory(_device->vkDevice, _stagingBufferMemory);

        CopyBuffer(_stagingBuffer, _vertexBuffer, totalBufferSize);

        vkCmdBindPipeline(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

        VkBuffer vertexBuffers[] = {_vertexBuffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdDraw(_vkCommandBuffer, vertexCounts, 1, 0, 0);
    }

    vkCmdEndRenderPass(_vkCommandBuffer);

    if (vkEndCommandBuffer(_vkCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphore};
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_vkCommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_device->vkGraphicQueue, 1, &submitInfo, _inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapchains[] = {_vkSwapchain};
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(_device->vkPresentQueue, &presentInfo);
}

void TEForwardRenderer::Cleanup()
{

    vkFreeMemory(_device->vkDevice, _vertexBufferMemory, nullptr);
    vkDestroyBuffer(_device->vkDevice, _vertexBuffer, nullptr);

    vkFreeMemory(_device->vkDevice, _stagingBufferMemory, nullptr);
    vkDestroyBuffer(_device->vkDevice, _stagingBuffer, nullptr);

    vkFreeCommandBuffers(_device->vkDevice, _vkCommandPool, 1, &_vkCommandBuffer);
    vkDestroyCommandPool(_device->vkDevice, _vkCommandPool, nullptr);

    vkDestroySemaphore(_device->vkDevice, _imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(_device->vkDevice, _renderFinishedSemaphore, nullptr);
    vkDestroyFence(_device->vkDevice, _inFlightFence, nullptr);

    vkDestroyPipelineLayout(_device->vkDevice, _vkPipelineLayout, nullptr);

    for (auto iter = _pipelines.begin(); iter != _pipelines.end(); iter++)
        vkDestroyPipeline(_device->vkDevice, iter->second, nullptr);

    vkDestroyRenderPass(_device->vkDevice, _vkRenderPass, nullptr);

    for (VkShaderModule &shaderModule : _vkShaderModules)
    {
        vkDestroyShaderModule(_device->vkDevice, shaderModule, nullptr);
    }

    for (auto imageView : _vkImageViews)
    {
        vkDestroyImageView(_device->vkDevice, imageView, nullptr);
    }

    for (VkFramebuffer &framebuffer : _vkFramebuffers)
    {
        vkDestroyFramebuffer(_device->vkDevice, framebuffer, nullptr);
    }

    vkDestroySwapchainKHR(_device->vkDevice, _vkSwapchain, nullptr);
}
