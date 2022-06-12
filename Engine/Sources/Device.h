#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <memory>

class TEWindow;

class TEDevice
{
public:
    TEDevice(const VkInstance &vkInstance, std::shared_ptr<TEWindow> window);

    void Init();
    void Cleanup();

public:
    VkPhysicalDevice vkPhysicalDevice;
    VkDevice vkDevice;
    VkQueue vkGraphicQueue, vkPresentQueue;
    uint32_t graphicQueueFamilyIndex, presentQueueFamilyIndex;

private:
    std::shared_ptr<TEWindow> _window;
    VkInstance _vkInstance;
};