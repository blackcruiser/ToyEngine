#pragma once

#include "CommonDefine.h"

#include <vulkan/vulkan.hpp>

class TEDevice;
class TECommandPool;

class TECommandBuffer
{
public:
    TECommandBuffer(TEPtr<TECommandPool> commandPool);

    void Init();
    void Cleanup();

    void Begin();
    void End();

    VkCommandBuffer GetRawCommandBuffer();

private:
    void CreateRawBuffer();
    void AllocateAndBindBufferMemory();

private:
    TEPtr<TECommandPool> _commandPool;

    VkCommandBuffer _vkCommandBuffer;
};