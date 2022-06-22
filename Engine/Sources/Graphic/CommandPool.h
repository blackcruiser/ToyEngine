#pragma once

#include "CommonDefine.h"

#include <vulkan/vulkan.hpp>

#include <set>

class TEDevice;
class TECommandBuffer;

class TECommandPool
{
public:
    TECommandPool(TEPtr<TEDevice> device);
    ~TECommandPool();

    TECommandBuffer *CreateCommandBuffer(TEPtr<TECommandPool> commandPool);
    void DestroyCommandBuffer(TECommandBuffer *commandBuffer);

    VkCommandPool GetRawCommandPool();
    TEPtr<TEDevice> GetDevice();

private:
    void _CreateRawCommandPool();

private:
    TEPtr<TEDevice> _device;

    VkCommandPool _vkCommandPool;
    std::set<TECommandBuffer *> _commandBuffers;
};