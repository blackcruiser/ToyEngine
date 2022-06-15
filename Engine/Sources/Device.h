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

public:
    VkPhysicalDevice vkPhysicalDevice;
    VkDevice vkDevice;
    VkQueue vkGraphicQueue, vkPresentQueue;
    uint32_t graphicQueueFamilyIndex, presentQueueFamilyIndex;

private:
    TEPtr<TEWindow> _window;
    VkInstance _vkInstance;
};