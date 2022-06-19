#include "GPU.h"
#include "Surface.h"
#include "device.h"

#include <stdexcept>
#include <set>


TEGPU::TEGPU(const VkInstance &vkInstance) : _vkInstance(vkInstance),
                                             _GPU(VK_NULL_HANDLE), _deviceExtensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME})
{
}

void TEGPU::Init()
{
    std::vector<VkPhysicalDevice> GPUs = GetSupportedGPUs();


    _GPU = VK_NULL_HANDLE;
    for (size_t i = 0; i < GPUs.size(); i++)
    {
        std::vector<VkExtensionProperties> availableExtensions = GetExtensionProperties(GPUs[i]);

        std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());

        for (const auto &extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        if (requiredExtensions.empty())
        {
            _GPU = GPUs[i];
            break;
        }
    }
}

void TEGPU::Cleanup()
{
}

VkPhysicalDevice TEGPU::GetRawPhysicalDevice()
{
    return _GPU;
}

const std::vector<const char *> &TEGPU::GetExtensions()
{
    return _deviceExtensions;
}

std::vector<VkExtensionProperties> TEGPU::GetExtensionProperties(VkPhysicalDevice GPU)
{
    uint32_t extensionCount = 0;     
    vkEnumerateDeviceExtensionProperties(GPU, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(GPU, nullptr, &extensionCount, availableExtensions.data());

    return availableExtensions;
}

std::vector<VkQueueFamilyProperties> TEGPU::GetQueueFamilyProperties()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_GPU, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilieProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_GPU, &queueFamilyCount, queueFamilieProperties.data());

    return queueFamilieProperties;
}

bool TEGPU::isSurfaceSupported(uint32_t queueFamilyIndex, TEPtr<TESurface> surface)
{
    VkBool32 isSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(_GPU, queueFamilyIndex, surface->GetRawSurface(), &isSupported);
    return isSupported != 0;
}

uint32_t TEGPU::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_GPU, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

TEPtr<TEDevice> TEGPU::CreateDevice(TEPtr<TEGPU> GPU, TEPtr<TESurface> surface)
{
    TEPtr<TEDevice> device = std::make_shared<TEDevice>(GPU, surface);
    device->Init();

    return device;
}

std::vector<VkPhysicalDevice> TEGPU::GetSupportedGPUs()
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

    return devices;
}
