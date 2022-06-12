#include "Device.h"
#include "Window.h"

#include <limits>
#include <stdexcept>
#include <vector>
#include <string>
#include <set>

TEDevice::TEDevice(const VkInstance &vkInstance, std::shared_ptr<TEWindow> window) : _window(window), _vkInstance(vkInstance),
                                                                                     vkPhysicalDevice(VK_NULL_HANDLE), vkDevice(VK_NULL_HANDLE), vkGraphicQueue(VK_NULL_HANDLE), vkPresentQueue(VK_NULL_HANDLE), graphicQueueFamilyIndex(std::numeric_limits<uint32_t>::max()), presentQueueFamilyIndex(std::numeric_limits<uint32_t>::max())
{
}

void TEDevice::Init()
{
    // Physical Device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("no physiccal device!");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, devices.data());

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    vkPhysicalDevice = VK_NULL_HANDLE;
    for (size_t i = 0; i < devices.size(); i++)
    {
        const VkPhysicalDevice &vkCurrentDevice = devices[i];
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(vkCurrentDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(vkCurrentDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        if (requiredExtensions.empty())
        {
            vkPhysicalDevice = devices[i];
            break;
        }
    }

    // Queue
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

    for (size_t i = 0; i < queueFamilies.size(); i++)
    {
        if (graphicQueueFamilyIndex == std::numeric_limits<uint32_t>::max() && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphicQueueFamilyIndex = i;

        VkBool32 isSupported;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, _window->vkSurface, &isSupported);
        if (presentQueueFamilyIndex == std::numeric_limits<uint32_t>::max() && isSupported)
            presentQueueFamilyIndex = i;
    }

    if (graphicQueueFamilyIndex >= queueFamilies.size())
    {
        throw std::runtime_error("can't find Device Queue!");
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {graphicQueueFamilyIndex, presentQueueFamilyIndex};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo vkDeviceCreateInfo{};
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    vkDeviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    vkDeviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    vkDeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    vkDeviceCreateInfo.enabledExtensionCount = deviceExtensions.size();

    if (vkCreateDevice(vkPhysicalDevice, &vkDeviceCreateInfo, nullptr, &vkDevice) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("create device fail!");
    }

    // Queue
    vkGetDeviceQueue(vkDevice, graphicQueueFamilyIndex, 0, &vkGraphicQueue);
    vkGetDeviceQueue(vkDevice, presentQueueFamilyIndex, 0, &vkPresentQueue);
}

void TEDevice::Cleanup()
{
    if (vkDevice != VK_NULL_HANDLE)
        vkDestroyDevice(vkDevice, nullptr);
}