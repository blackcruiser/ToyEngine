#pragma once

#include "CommonDefine.h"
#include "Device.h"
#include "Window.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

class TEScene;
class TEObject;
class TEMaterial;
class TECommandPool;
class TECommandBuffer;

class TERendererInterface
{
public:
	virtual void Init() = 0;
	virtual void RenderFrame(TEPtr<TEScene> scene) = 0;
	virtual void Cleanup() = 0;

private:
};

class TEForwardRenderer : public TERendererInterface
{
public:
	TEForwardRenderer(TEPtr<TEDevice> device, TEPtr<TESurface> surface);
	virtual void Init() override;

	VkPipeline CreatePipeline(TEPtr<TEMaterial> material);
	void CreateSwapchain(VkRenderPass renderPass);

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void GatherObjects(TEPtr<TEScene> scene);
	virtual void RenderFrame(TEPtr<TEScene> scene) override;

	virtual void Cleanup() override;

private:
	std::map<std::uintptr_t, TEPtrArr<TEObject>> _objectsToRender;

	TEPtr<TEDevice> _device;
	TEPtr<TESurface> _surface;

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

	TEPtr<TECommandPool> _commandPool;
	TECommandBuffer *_commandBuffer;
};