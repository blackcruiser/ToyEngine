#pragma once

#include "CoreDefine.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class TEGPU;
class TESurface;

class TEDevice
{
public:
    TEDevice(TEPtr<TEGPU> GPU, TEPtr<TESurface> surface);
    ~TEDevice();

    VkBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    void DestroyBuffer(VkBuffer buffer);

    VkDeviceMemory AllocateAndBindBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties);
    void FreeMemmory(VkDeviceMemory deviceMemory);

    VkSemaphore CreateSemaphore();
    void DestroySemaphore(VkSemaphore semaphore);

    VkFence CreateFence(bool isSignaled);
    void DestroyFence(VkFence fence);

    VkShaderModule CreateShaderModule(const std::vector<char> &code);
    void DestroyShaderModule(VkShaderModule shaderModule);

    VkImageView CreateImageView(VkImage image, VkFormat format);
    void DestroyImageView(VkImageView imageView);

    VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView> imageViewArr, uint32_t width, uint32_t height);
    void DestroyFramebuffer(VkFramebuffer framebuffer);

    VkSwapchainKHR CreateSwapchain(uint32_t imageCount, VkFormat imageFormat, VkColorSpaceKHR colorSpace, VkExtent2D extent, VkSurfaceTransformFlagBitsKHR preTransform, VkPresentModeKHR presentMode);
    void DestroySwapchain(VkSwapchainKHR swapchain);

    VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout);
    void DestroyPipelineLayout(VkPipelineLayout pipelineLayout);

    VkPipeline CreateGraphicPipeline(VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule, VkExtent2D extent, VkPipelineLayout pipelineLayout, VkRenderPass renderPass);
    void DestroyPipeline(VkPipeline pipeline);

    VkRenderPass CreateRenderPass(VkFormat format);
    void DestroyRenderPass(VkRenderPass renderPass);

    VkDescriptorSetLayout CreateDescriptorSetLayout(VkDescriptorType type, uint32_t descriptorCount, VkShaderStageFlags stageFlags);
    void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);

    VkDescriptorPool CreateDescriptorPool(VkDescriptorType type, uint32_t descriptorCount);
    void DestroyDescriptorPool(VkDescriptorPool descriptorPool);

    VkDescriptorSet AllocateDescriptorSet(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSetLayout *pSetLayouts);

    void UpdateDescriptorSet(VkDescriptorSet descriptorSet, VkDescriptorType descriptorType, VkBuffer buffer, VkDeviceSize size);

    void WaitIdle();

    VkDevice GetRawDevice();

    VkQueue GetGraphicQueue();
    VkQueue GetPresentQueue();

    uint32_t GetGraphicQueueFamilyIndex();
    uint32_t GetPresentQueueFamilyIndex();

private:
    VkDevice _vkDevice;
    VkQueue _vkGraphicQueue, _vkPresentQueue;
    uint32_t _graphicQueueFamilyIndex, _presentQueueFamilyIndex;

    TEPtr<TEGPU> _GPU;
    TEPtr<TESurface> _surface;
};