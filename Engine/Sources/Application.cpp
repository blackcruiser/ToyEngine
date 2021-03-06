#include "Application.h"
#include "Render/Renderer.h"
#include "Graphic/GPU.h"
#include "Graphic/Window.h"
#include "Graphic/Device.h"
#include "Graphic/Surface.h"
#include "Input/InputSystem.h"
#include "Scene/Scene.h"

#include <string>
#include <stdexcept>
#include <set>

const std::string AppName("ToyEngine");
const int kWidth = 800;
const int kHeight = 800;

TEApplication::TEApplication() : _device(nullptr), _window(nullptr), _renderer(nullptr)
{

    _InitGlfw();
    _CreateVulkanInstance();

    _window = std::make_shared<TEWindow>(AppName, kWidth, kHeight);
    _window->RegisterInput(TEInputSystem::GetInstance());

    _GPU = std::make_shared<TEGPU>(_vkInstance);
    _surface = std::make_shared<TESurface>(_vkInstance, _GPU, _window);
    _device = std::make_shared<TEDevice>(_GPU, _surface);
    _renderer = std::make_shared<TEForwardRenderer>(_device, _surface);
}

TEApplication::~TEApplication()
{
    _renderer.reset();
    _device.reset();
    _surface.reset();
    _GPU.reset();

    _window->UnregisterInput(TEInputSystem::GetInstance());
    _window.reset();

    _CleanupVulkan();
    _CleanupGlfw();
}

void TEApplication::_InitGlfw()
{
    glfwInit();
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

void TEApplication::Run(TEPtr<TEScene> scene)
{
    while (!_window->ShouldClose())
    {
        glfwPollEvents();

        _renderer->RenderFrame(scene);
    }

    _device->WaitIdle();
}

void TEApplication::_CleanupVulkan()
{

    if (_vkInstance != VK_NULL_HANDLE)
        vkDestroyInstance(_vkInstance, nullptr);
}

void TEApplication::_CleanupGlfw()
{
    glfwTerminate();
}