#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Device.h"

TECommandPool::TECommandPool(TEPtr<TEDevice> device) : _device(device)
{
    _CreateRawCommandPool();
}

TECommandPool::~TECommandPool()
{


    VkDevice vkDevice = _device->GetRawDevice();
    vkDestroyCommandPool(vkDevice, _vkCommandPool, nullptr);
}

TECommandBuffer *TECommandPool::CreateCommandBuffer(TEPtr<TECommandPool> commandPool)
{
    TECommandBuffer *commandBuffer = new TECommandBuffer(commandPool);

    _commandBuffers.insert(commandBuffer);

    return commandBuffer;
}

void TECommandPool::DestroyCommandBuffer(TECommandBuffer *commandBuffer)
{
    _commandBuffers.erase(commandBuffer);
    delete commandBuffer;
}


VkCommandPool TECommandPool::GetRawCommandPool()
{
    return _vkCommandPool;
}

TEPtr<TEDevice> TECommandPool::GetDevice()
{
    return _device;
}

void TECommandPool::_CreateRawCommandPool()
{
    VkDevice vkDevice = _device->GetRawDevice();
    uint32_t graphicQueueFamilyIndex = _device->GetGraphicQueueFamilyIndex();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicQueueFamilyIndex;

    if (vkCreateCommandPool(vkDevice, &poolInfo, nullptr, &_vkCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}
