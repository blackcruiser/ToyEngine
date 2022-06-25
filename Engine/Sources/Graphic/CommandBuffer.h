#pragma once

#include "CoreDefine.h"

#include <vulkan/vulkan.hpp>

class TEDevice;
class TECommandPool;

class TECommandBuffer
{
public:
    TECommandBuffer(TEPtr<TECommandPool> commandPool);
    ~TECommandBuffer();

    void Begin();
    void End();

    VkCommandBuffer GetRawCommandBuffer();

private:
    void _CreateRawBuffer();
    void _AllocateAndBindBufferMemory();

private:
    TEPtr<TECommandPool> _commandPool;

    VkCommandBuffer _vkCommandBuffer;
};