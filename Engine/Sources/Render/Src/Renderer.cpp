#include "Renderer.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"
#include "Scene/MeshComponent.h"
#include "Scene/MaterialComponent.h"
#include "Scene/CameraComponent.h"
#include "Scene/TransformComponent.h"
#include "Graphic/Surface.h"
#include "Graphic/CommandPool.h"
#include "Graphic/CommandBuffer.h"

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <array>
#include <cstddef>
#include <functional>

static std::vector<char> readFile(const std::string& filename)
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

TEForwardRenderer::TEForwardRenderer(TEPtr<TEDevice> device, TEPtr<TESurface> surface) : _device(device), _surface(surface), _stagingBuffer(VK_NULL_HANDLE), _stagingBufferSize(0), _vertexBuffer(VK_NULL_HANDLE), _vertexBufferSize(0), _indexBuffer(VK_NULL_HANDLE), _indexesBufferSize(0), _uniformBuffer(VK_NULL_HANDLE)
{
    VkSurfaceFormatKHR surfaceFormat = _surface->GetSurfaceFormat();
    _vkRenderPass = _device->CreateRenderPass(surfaceFormat.format);
    CreateSwapchain(_vkRenderPass);

    _commandPool = std::make_shared<TECommandPool>(_device);
    _commandBuffer = _commandPool->CreateCommandBuffer(_commandPool);

    _imageAvailableSemaphore = _device->CreateSemaphore();
    _renderFinishedSemaphore = _device->CreateSemaphore();

    _inFlightFence = _device->CreateFence(true);

    _descriptorPool = _device->CreateDescriptorPool(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    _descriptorLayout = _device->CreateDescriptorSetLayout(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);

    _uniformBuffer = _device->CreateBuffer(sizeof(glm::mat4x4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    _uniformBufferMemory = _device->AllocateAndBindBufferMemory(_uniformBuffer,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    _descriptorSet = _device->AllocateDescriptorSet(_descriptorPool, 1, &_descriptorLayout);
    _device->UpdateDescriptorSet(_descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _uniformBuffer, sizeof(glm::mat4x4));
}

TEForwardRenderer::~TEForwardRenderer()
{
    _device->FreeMemmory(_uniformBufferMemory);
    _device->DestroyBuffer(_uniformBuffer);

    _device->FreeMemmory(_indexBufferMemory);
    _device->DestroyBuffer(_indexBuffer);

    _device->FreeMemmory(_vertexBufferMemory);
    _device->DestroyBuffer(_vertexBuffer);

    _device->FreeMemmory(_stagingBufferMemory);
    _device->DestroyBuffer(_stagingBuffer);

    _device->DestroyDescriptorSetLayout(_descriptorLayout);
    _device->DestroyDescriptorPool(_descriptorPool);

    _device->DestroySemaphore(_imageAvailableSemaphore);
    _device->DestroySemaphore(_renderFinishedSemaphore);
    _device->DestroyFence(_inFlightFence);

    _device->DestroyPipelineLayout(_vkPipelineLayout);

    for (auto iter = _pipelines.begin(); iter != _pipelines.end(); iter++)
        _device->DestroyPipeline(iter->second);

    _device->DestroyRenderPass(_vkRenderPass);

    for (VkShaderModule& shaderModule : _vkShaderModules)
    {
        _device->DestroyShaderModule(shaderModule);
    }

    for (auto imageView : _vkImageViews)
    {
        _device->DestroyImageView(imageView);
    }

    for (VkFramebuffer& framebuffer : _vkFramebuffers)
    {
        _device->DestroyFramebuffer(framebuffer);
    }

    _commandPool->DestroyCommandBuffer(_commandBuffer);
    _commandPool.reset();
    _device->DestroySwapchain(_vkSwapchain);
}

VkPipeline TEForwardRenderer::CreatePipeline(TEPtr<TEMaterialComponent> material)
{
    auto vertShaderCode = readFile("Shaders/VertexShader.spv");
    auto fragShaderCode = readFile("Shaders/FragmentShader.spv");

    // Shader
    VkShaderModule vkVerterShaderModule = _device->CreateShaderModule(vertShaderCode);
    _vkShaderModules.push_back(vkVerterShaderModule);
    VkShaderModule vkFragmentShaderModule = _device->CreateShaderModule(fragShaderCode);
    _vkShaderModules.push_back(vkFragmentShaderModule);

    _vkPipelineLayout = _device->CreatePipelineLayout(_descriptorLayout);
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
            _vkImageViews[i] };

        _vkFramebuffers[i] = _device->CreateFramebuffer(renderPass, attachments, extent.width, extent.height);
    }
}

void TEForwardRenderer::GatherObjects(TEPtr<TEScene> scene)
{
    const TEPtrArr<TESceneObject>& objects = scene->GetObjects();
    std::hash<TEMaterialComponent*> hashCreator;

    _objectsToRender.clear();
    for (auto& object : objects)
    {
        TEPtr<TEMaterialComponent> material = object->GetComponent<TEMaterialComponent>();
        size_t address = hashCreator(material.get());
        if (_objectsToRender.find(address) == _objectsToRender.end())
            _objectsToRender.emplace(address, TEPtrArr<TESceneObject>());
        TEPtrArr<TESceneObject>& objectArr = _objectsToRender.at(address);
        objectArr.push_back(object);
    }
}

void TEForwardRenderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    TECommandBuffer* commandBuffer = _commandPool->CreateCommandBuffer(_commandPool);
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
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = _surface->GetExtent();

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    VkCommandBuffer vkCommandBuffer = _commandBuffer->GetRawCommandBuffer();
    _commandBuffer->Begin();

    vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    TEPtr<TECameraComponent> cameraComponent = scene->GetCamera();
    glm::mat4x4 VP = cameraComponent->GetProjectMatrix() * cameraComponent->GetViewMatrix();

    for (auto& pair : _objectsToRender)
    {
        auto& objectArr = pair.second;
        if (objectArr.empty())
            continue;

        TEPtr<TEMaterialComponent> materialComponent = objectArr[0]->GetComponent<TEMaterialComponent>();

        VkPipeline vkPipeline;

        std::hash<TEMaterialComponent*> hashCreator;
        size_t address = hashCreator(materialComponent.get());
        if (_pipelines.find(address) == _pipelines.end())
        {
            vkPipeline = CreatePipeline(materialComponent);
            _pipelines.insert(std::make_pair(address, vkPipeline));
        }
        else
        {
            vkPipeline = _pipelines[address];
        }

        vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

        for (auto& object : objectArr)
        {
            TEPtr<TEMeshComponent> meshComponent = object->GetComponent<TEMeshComponent>();

            if (meshComponent == nullptr)
                continue;

            const std::vector<glm::vec3>& vertices = meshComponent->GetVertices();
            const std::vector<int>& indexes = meshComponent->GetIndexes();

            TEPtr<TETransformComponent> transformComponent = object->GetComponent<TETransformComponent>();
            glm::mat4x4 MVP = VP * transformComponent->GetTransform();

            size_t curVertexBufferSize = vertices.size() * sizeof(glm::vec3);
            size_t curIndexBufferSize = indexes.size() * sizeof(int);

            if (_stagingBuffer != VK_NULL_HANDLE && (curVertexBufferSize > _stagingBufferSize || curIndexBufferSize > _stagingBufferSize))
            {
                _device->FreeMemmory(_stagingBufferMemory);
                _device->DestroyBuffer(_stagingBuffer);

                _stagingBuffer = VK_NULL_HANDLE;
            }

            if (_stagingBuffer == VK_NULL_HANDLE)
            {
                _stagingBufferSize = std::max(curIndexBufferSize, curVertexBufferSize);
                _stagingBuffer = _device->CreateBuffer(_stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
                _stagingBufferMemory = _device->AllocateAndBindBufferMemory(_stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            }

            if (_vertexBuffer != VK_NULL_HANDLE && curVertexBufferSize > _vertexBufferSize)
            {
                _device->FreeMemmory(_vertexBufferMemory);
                _device->DestroyBuffer(_vertexBuffer);

                _vertexBuffer = VK_NULL_HANDLE;
            }

            if (_vertexBuffer == VK_NULL_HANDLE)
            {
                _vertexBufferSize = curVertexBufferSize;
                _vertexBuffer = _device->CreateBuffer(curVertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
                _vertexBufferMemory = _device->AllocateAndBindBufferMemory(_vertexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            }

            if (_indexBuffer != VK_NULL_HANDLE && curIndexBufferSize > _indexesBufferSize)
            {
                _device->FreeMemmory(_indexBufferMemory);
                _device->DestroyBuffer(_indexBuffer);

                _indexBuffer = VK_NULL_HANDLE;
            }

            if (_indexBuffer == VK_NULL_HANDLE)
            {
                _indexesBufferSize = curIndexBufferSize;
                _indexBuffer = _device->CreateBuffer(curIndexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
                _indexBufferMemory = _device->AllocateAndBindBufferMemory(_indexBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            }

            void* verticesDataPtr = nullptr;
            vkMapMemory(_device->GetRawDevice(), _stagingBufferMemory, 0, curVertexBufferSize, 0, &verticesDataPtr);
            memcpy(verticesDataPtr, vertices.data(), curVertexBufferSize);
            vkUnmapMemory(_device->GetRawDevice(), _stagingBufferMemory);

            CopyBuffer(_stagingBuffer, _vertexBuffer, curVertexBufferSize);


            void* indexesDataPtr = nullptr;
            vkMapMemory(_device->GetRawDevice(), _stagingBufferMemory, 0, curIndexBufferSize, 0, &indexesDataPtr);
            memcpy(indexesDataPtr, indexes.data(), curIndexBufferSize);
            vkUnmapMemory(_device->GetRawDevice(), _stagingBufferMemory);

            CopyBuffer(_stagingBuffer, _indexBuffer, curIndexBufferSize);

            VkBuffer vertexBuffers[] = { _vertexBuffer };
            VkDeviceSize offsets[] = { 0 };

            vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(vkCommandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT32);

            void* uniformDataPtr = nullptr;
            vkMapMemory(_device->GetRawDevice(), _uniformBufferMemory, 0, sizeof(glm::mat4x4), 0, &uniformDataPtr);
            memcpy(uniformDataPtr, &MVP, sizeof(glm::mat4x4));
            vkUnmapMemory(_device->GetRawDevice(), _uniformBufferMemory);

            vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vkPipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);
            vkCmdDrawIndexed(vkCommandBuffer, indexes.size(), 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(vkCommandBuffer);

    _commandBuffer->End();

    VkSemaphore waitSemaphores[] = { _imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { _renderFinishedSemaphore };
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

    VkSwapchainKHR swapchains[] = { _vkSwapchain };
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
