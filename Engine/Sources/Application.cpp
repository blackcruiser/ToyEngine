#include "Application.h"
#include "Renderer.h"

#include <string>
#include <stdexcept>
#include <set>

const std::string AppName("ToyEngine");
const int kWidth = 800;
const int kHeight = 800;

TEApplication::TEApplication() : _device(nullptr), _window(nullptr), _renderer(nullptr)
{
}

void TEApplication::Init()
{
    _device = std::make_shared<TEDevice>();
    _window = std::make_shared<TEWindow>();

    _InitGlfw();
    _CreateVulkanInstance();
    _CreateSurface();
    _CreateVulkanDevice();

    _renderer = std::make_shared<TEForwardRenderer>();
    _renderer->Init(_device, _window);
}

void TEApplication::_InitGlfw()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window->glfwWindow = glfwCreateWindow(kWidth, kHeight, AppName.c_str(), nullptr, nullptr);

    if (_window->glfwWindow == nullptr)
    {
        throw std::runtime_error("failed to create window!");
        return;
    }
}

void TEApplication::_CreateVulkanInstance()
{
    // Instance
    VkApplicationInfo vkAppInfo{};
    vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkAppInfo.pApplicationName = AppName.c_str();
    vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.pEngineName = "No Engine";
    vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    const std::vector<const char *> validationLayers = {
#ifdef TOYENGINE_DEBUG
        "VK_LAYER_KHRONOS_validation"
#endif
    };

    VkInstanceCreateInfo vkInstanceCreateInfo{};
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pApplicationInfo = &vkAppInfo;
    vkInstanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = extensions.data();
    vkInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    vkInstanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

    if (vkCreateInstance(&vkInstanceCreateInfo, nullptr, &_vkInstance) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vulkan instance!");
        return;
    }
}

void TEApplication::_CreateSurface()
{
    // Surface
    if (glfwCreateWindowSurface(_vkInstance, _window->glfwWindow, nullptr, &_window->vkSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("can't Create Surface");
    }
}

void TEApplication::_CreateVulkanDevice()
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
    _device->vkPhysicalDevice = VK_NULL_HANDLE;
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
            _device->vkPhysicalDevice = devices[i];
            break;
        }
    }

    // Queue
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_device->vkPhysicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_device->vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

    uint32_t graphicQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
    uint32_t presentQueueFamilyIndex = std::numeric_limits<uint32_t>::max();
    for (size_t i = 0; i < queueFamilies.size(); i++)
    {
        if (graphicQueueFamilyIndex == std::numeric_limits<uint32_t>::max() && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphicQueueFamilyIndex = i;

        VkBool32 isSupported;
        vkGetPhysicalDeviceSurfaceSupportKHR(_device->vkPhysicalDevice, i, _window->vkSurface, &isSupported);
        if (presentQueueFamilyIndex == std::numeric_limits<uint32_t>::max() && isSupported)
            presentQueueFamilyIndex = i;
    }

    if (graphicQueueFamilyIndex >= queueFamilies.size())
    {
        throw std::runtime_error("can't find Device Queue!");
    }
    _device->graphicQueueFamilyIndex = graphicQueueFamilyIndex;
    _device->presentQueueFamilyIndex = presentQueueFamilyIndex;

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

    if (vkCreateDevice(_device->vkPhysicalDevice, &vkDeviceCreateInfo, nullptr, &_device->vkDevice) != VkResult::VK_SUCCESS)
    {
        throw std::runtime_error("create device fail!");
    }

    // Queue
    vkGetDeviceQueue(_device->vkDevice, graphicQueueFamilyIndex, 0, &_device->vkGraphicQueue);
    vkGetDeviceQueue(_device->vkDevice, presentQueueFamilyIndex, 0, &_device->vkPresentQueue);
}

void TEApplication::Run()
{
    while (!glfwWindowShouldClose(_window->glfwWindow))
    {
        glfwPollEvents();

        _renderer->RenderFrame();
    }

    vkDeviceWaitIdle(_device->vkDevice);
}

void TEApplication::Cleanup()
{
    _renderer->Cleanup();

    vkDestroySurfaceKHR(_vkInstance, _window->vkSurface, nullptr);

    _CleanupVulkan();
    _CleanupGlfw();
}

void TEApplication::_CleanupVulkan()
{
    if (_device->vkDevice != VK_NULL_HANDLE)
        vkDestroyDevice(_device->vkDevice, nullptr);

    if (_vkInstance != VK_NULL_HANDLE)
        vkDestroyInstance(_vkInstance, nullptr);
}

void TEApplication::_CleanupGlfw()
{
    glfwDestroyWindow(_window->glfwWindow);

    glfwTerminate();
}