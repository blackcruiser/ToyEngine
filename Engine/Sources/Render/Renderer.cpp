#include "Renderer.h"
#include "Scene.h"
#include "Object.h"
#include "Material.h"
#include "Surface.h"
#include "CommandPool.h"
#include "CommandBuffer.h"

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

TEForwardRenderer::TEForwardRenderer(TEPtr<TEDevice> device, TEPtr<TESurface> surface) : _device(device), _surface(surface), _stagingBuffer(VK_NULL_HANDLE), _stagingBufferSize(0), _vertexBuffer(VK_NULL_HANDLE), _vertexBufferSize(0)
{
}

void TEForwardRenderer::Init()
{
    VkSurfaceFormatKHR surfaceFormat = _surface->GetSurfaceFormat();
    _vkRenderPass = _device->CreateRenderPass(surfaceFormat.format);
    CreateSwapchain(_vkRenderPass);

    _commandPool = _device->CreateCommandPool(_device);
    _commandBuffer = _commandPool->CreateCommandBuffer(_commandPool);

    _imageAvailableSemaphore = _device->CreateSemaphore();
    _renderFinishedSemaphore = _device->CreateSemaphore();

    _inFlightFence = _device->CreateFence(true);
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
    VkPipeline vkPipeline = _device->CreateGraphicPipeline(vkVerterShaderModule, vkFragmentShaderModule, _surface->GetExtent(), _vkPipelineLayout, _vkRenderPass);

    return vkPipeline;
}

void TEForwardRenderer::CreateSwapchain(VkRenderPass renderPass)
{
    VkSurfaceFormatKHR vkSurfaceFormat = _surface->GetSurfaceFormat();
    VkPresentModeKHR vkPresentMode = _surface->GetPresentMode();
    VkSurfaceCapabilitiesKHR vkCapabilities = _surface->GetCpabilities();
    VkExtent2D extent = _surface->GetExtent();

    _vkSwapchain = _device->CreateSwapchain(vkCapabilities.minImageCount + 1, vkSurfaceFormat.format, vkSurfaceFormat.colorSpace, extent, vkCapabilities.currentTransform, vkPresentMode);

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(_device->GetRawDevice(), _vkSwapchain, &imageCount, nullptr);
    _vkImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device->GetRawDevice(), _vkSwapchain, &imageCount, _vkImages.data());

    // Image View
    _vkImageViews.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        _vkImageViews[i] = _device->CreateImageView(_vkImages[i], vkSurfaceFormat.format);
    }

    _vkFramebuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        std::vector<VkImageView> attachments = {
            _vkImageViews[i]};

        _vkFramebuffers[i] = _device->CreateFramebuffer(renderPass, attachments, extent.width, extent.height);
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
    TECommandBuffer *commandBuffer = _commandPool->CreateCommandBuffer(_commandPool);
    VkCommandBuffer vkCommandBuffer = commandBuffer->GetRawCommandBuffer();

    commandBuffer->Begin();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(vkCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    commandBuffer->End();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCommandBuffer;

    vkQueueSubmit(_device->GetGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_device->GetGraphicQueue());

    _commandPool->DestroyCommandBuffer(commandBuffer);
}

void TEForwardRenderer::RenderFrame(TEPtr<TEScene> scene)
{
    GatherObjects(scene);

    VkDevice vkDevice = _device->GetRawDevice();
    vkWaitForFences(vkDevice, 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(vkDevice, 1, &_inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(vkDevice, _vkSwapchain, UINT64_MAX, _imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _vkRenderPass;
    renderPassInfo.framebuffer = _vkFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _surface->GetExtent();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    VkCommandBuffer vkCommandBuffer = _commandBuffer->GetRawCommandBuffer();
    _commandBuffer->Begin();

    vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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
            _device->FreeMemmory(_stagingBufferMemory);
            _device->DestroyBuffer(_stagingBuffer);

            _stagingBuffer = VK_NULL_HANDLE;
        }

        if (_stagingBuffer == VK_NULL_HANDLE)
        {
            _stagingBuffer = _device->CreateBuffer(totalBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
            _stagingBufferMemory = _device->AllocateAndBindBufferMemory(_stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            _stagingBufferSize = totalBufferSize;
        }

        if (_vertexBuffer != VK_NULL_HANDLE && totalBufferSize > _vertexBufferSize)
        {
            _device->FreeMemmory(_vertexBufferMemory);
            _device->DestroyBuffer(_vertexBuffer);

            _vertexBuffer = VK_NULL_HANDLE;
        }

        if (_vertexBuffer == VK_NULL_HANDLE)
        {
            _vertexBuffer = _device->CreateBuffer(totalBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            _vertexBufferMemory = _device->AllocateAndBindBufferMemory(_vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }

        void *data;
        vkMapMemory(_device->GetRawDevice(), _stagingBufferMemory, 0, totalBufferSize, 0, &data);

        uint8_t *dataPtr = reinterpret_cast<uint8_t *>(data);
        uint32_t vertexCounts = 0;
        for (auto &object : objectArr)
        {
            size_t bufferSize = object->vertices.size() * sizeof(glm::vec3);
            memcpy(dataPtr, object->vertices.data(), bufferSize);
            dataPtr = dataPtr + bufferSize;
            vertexCounts += object->vertices.size();
        }

        vkUnmapMemory(_device->GetRawDevice(), _stagingBufferMemory);

        CopyBuffer(_stagingBuffer, _vertexBuffer, totalBufferSize);

        vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

        VkBuffer vertexBuffers[] = {_vertexBuffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdDraw(vkCommandBuffer, vertexCounts, 1, 0, 0);
    }

    vkCmdEndRenderPass(vkCommandBuffer);

    _commandBuffer->End();

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphore};
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_device->GetGraphicQueue(), 1, &submitInfo, _inFlightFence) != VK_SUCCESS)
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

    vkQueuePresentKHR(_device->GetPresentQueue(), &presentInfo);
}

void TEForwardRenderer::Cleanup()
{
    _device->FreeMemmory(_vertexBufferMemory);
    _device->DestroyBuffer(_vertexBuffer);

    _device->FreeMemmory(_stagingBufferMemory);
    _device->DestroyBuffer(_stagingBuffer);

    _commandBuffer->Cleanup();
    _commandPool->Cleanup();

    _device->DestroySemaphore(_imageAvailableSemaphore);
    _device->DestroySemaphore(_renderFinishedSemaphore);
    _device->DestroyFence(_inFlightFence);

    _device->DestroyPipelineLayout(_vkPipelineLayout);

    for (auto iter = _pipelines.begin(); iter != _pipelines.end(); iter++)
        _device->DestroyPipeline(iter->second);

    _device->DestroyRenderPass(_vkRenderPass);

    for (VkShaderModule &shaderModule : _vkShaderModules)
    {
        _device->DestroyShaderModule(shaderModule);
    }

    for (auto imageView : _vkImageViews)
    {
        _device->DestroyImageView(imageView);
    }

    for (VkFramebuffer &framebuffer : _vkFramebuffers)
    {
        _device->DestroyFramebuffer(framebuffer);
    }

    _device->DestroySwapchain(_vkSwapchain);
}
