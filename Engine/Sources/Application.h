#pragma once

#include "CoreDefine.h"

#include <vulkan/vulkan.hpp>

#include <memory>

class TEGPU;
class TEDevice;
class TEWindow;
class TESurface;
class TERendererInterface;

class TEApplication
{
public:
    TEApplication();
    ~TEApplication();

    void Run();

private:
    void _InitGlfw();
    void _CreateVulkanInstance();

    void _CleanupGlfw();
    void _CleanupVulkan();

private:
    TEPtr<TEGPU> _GPU;
    TEPtr<TEDevice> _device;
    TEPtr<TEWindow> _window;
    TEPtr<TESurface> _surface;
    TEPtr<TERendererInterface> _renderer;

    VkInstance _vkInstance;
};