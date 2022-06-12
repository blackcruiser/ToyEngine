#pragma once

#include "CommonDefine.h"
#include "Window.h"
#include "Device.h"

#include <memory>

class TERendererInterface;

class TEApplication
{
public:
    TEApplication();

    void Init();
    void Run();
    void Cleanup();

private:
    void _InitGlfw();
    void _CreateVulkanInstance();
    void _CreateSurface();
    void _CreateVulkanDevice();

    void _CleanupGlfw();
    void _CleanupVulkan();

private:
    std::shared_ptr<TEDevice> _device;
    std::shared_ptr<TEWindow> _window;
    std::shared_ptr<TERendererInterface> _renderer;

    VkInstance _vkInstance;
};