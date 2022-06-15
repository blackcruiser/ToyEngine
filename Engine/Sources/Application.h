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

    void _CleanupGlfw();
    void _CleanupVulkan();

private:
    TEPtr<TEDevice> _device;
    TEPtr<TEWindow> _window;
    TEPtr<TERendererInterface> _renderer;

    VkInstance _vkInstance;
};