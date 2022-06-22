#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Device.h"

TECommandBuffer::TECommandBuffer(TEPtr<TECommandPool> commandPool) : _commandPool(commandPool)
{

    _CreateRawBuffer();
}

TECommandBuffer::~TECommandBuffer()
{
    VkDevice vkDevice = _commandPool->GetDevice()->GetRawDevice();
    VkCommandPool vkCommandPool = _commandPool->GetRawCommandPool();

    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &_vkCommandBuffer);
}

void TECommandBuffer::Begin()
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(_vkCommandBuffer, &beginInfo);
}

void TECommandBuffer::End()
{
    vkEndCommandBuffer(_vkCommandBuffer);
}

VkCommandBuffer TECommandBuffer::GetRawCommandBuffer()
{
    return _vkCommandBuffer;
}

void TECommandBuffer::_CreateRawBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool->GetRawCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    _vkCommandBuffer;
    VkDevice vkDevice = _commandPool->GetDevice()->GetRawDevice();
    if (vkAllocateCommandBuffers(vkDevice, &allocInfo, &_vkCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
