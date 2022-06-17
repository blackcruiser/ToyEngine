#pragma once

#include "CommonDefine.h"
#include "Device.h"
#include "Window.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

class TEScene;
class TEObject;
class TEMaterial;

class TERendererInterface
{
public:
	virtual void Init(TEPtr<TEDevice> device, TEPtr<TEWindow> window) = 0;
	virtual void RenderFrame(TEPtr<TEScene> scene) = 0;
	virtual void Cleanup() = 0;

private:
};

class TEForwardRenderer : public TERendererInterface
{
public:
	virtual void Init(TEPtr<TEDevice> device, TEPtr<TEWindow> window) override;
	void SelectSurfaceFormat();
	void CreateRenderPass();
	VkPipeline CreatePipeline(TEPtr<TEMaterial> material);
	void CreateSwapchain(VkRenderPass renderPass);

	void CreateCommandPool();
	void CreateCommandBuffer();

	void CreateSemaphore();

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	VkShaderModule CreateShaderModule(const std::vector<char> &code);

	void GatherObjects(TEPtr<TEScene> scene);
	virtual void RenderFrame(TEPtr<TEScene> scene) override;

	virtual void Cleanup() override;

private:
	std::map<std::uintptr_t, TEPtrArr<TEObject>> _objectsToRender;

	TEPtr<TEDevice> _device;
	TEPtr<TEWindow> _window;

	VkSurfaceFormatKHR _vkSurfaceFormat;
	VkExtent2D _vkExtent;
	VkSwapchainKHR _vkSwapchain;
	std::vector<VkFramebuffer> _vkFramebuffers;
	std::vector<VkImage> _vkImages;
	std::vector<VkImageView> _vkImageViews;
	std::vector<VkShaderModule> _vkShaderModules;

	VkRenderPass _vkRenderPass;
	VkPipelineLayout _vkPipelineLayout;
	std::map<std::uintptr_t, VkPipeline> _pipelines;

	VkBuffer _stagingBuffer, _vertexBuffer;
	VkDeviceMemory _stagingBufferMemory, _vertexBufferMemory;
	size_t _stagingBufferSize, _vertexBufferSize;

	VkSemaphore _imageAvailableSemaphore, _renderFinishedSemaphore;
	VkFence _inFlightFence;

	VkCommandPool _vkCommandPool;
	VkCommandBuffer _vkCommandBuffer;
};