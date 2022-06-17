#pragma once

#include "CommonDefine.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class TEWindow;

class TEDevice
{
public:
    TEDevice(const VkInstance &vkInstance, TEPtr<TEWindow> window);

    void Init();
    void Cleanup();

    VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex);
    VkCommandBuffer CreateCommandBuffer(const VkCommandPool &commandPool);

    VkBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    VkDeviceMemory AllocateAndBindBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkSemaphore CreateSemaphore();
    VkFence CreateFence(bool isSignaled);

    VkShaderModule CreateShaderModule(const std::vector<char> &code);

    VkImageView CreateImageView(VkImage image, VkFormat format);

    VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView> imageViewArr, uint32_t width, uint32_t height);

    VkSwapchainKHR CreateSwapchain(uint32_t imageCount, VkFormat imageFormat, VkColorSpaceKHR colorSpace, VkExtent2D extent, VkSurfaceTransformFlagBitsKHR preTransform, VkPresentModeKHR presentMode);

    VkPipelineLayout CreatePipelineLayout();
    VkPipeline CreateGraphicPipeline(VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule, VkExtent2D extent, VkPipelineLayout pipelineLayout, VkRenderPass renderPass);

    VkRenderPass CreateRenderPass(VkFormat format);

public:
    VkPhysicalDevice vkPhysicalDevice;
    VkDevice vkDevice;
    VkQueue vkGraphicQueue, vkPresentQueue;
    uint32_t graphicQueueFamilyIndex, presentQueueFamilyIndex;

private:
    TEPtr<TEWindow> _window;
    VkInstance _vkInstance;
};