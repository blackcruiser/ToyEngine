#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


class TEDevice
{
public:
    VkPhysicalDevice vkPhysicalDevice;
    VkDevice vkDevice;
    VkQueue vkGraphicQueue, vkPresentQueue;
    uint32_t graphicQueueFamilyIndex, presentQueueFamilyIndex;
};