#pragma once

#include "Device.h"
#include "Window.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <map>

class TEScene;
class TEObject;

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
	void CreateMainPipeline();
	void CreateSwapchain(VkRenderPass renderPass);

	void CreateCommandPool();
	void CreateCommandBuffer();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	VkShaderModule CreateShaderModule(const std::vector<char> &code);

	void GatherObjects(TEPtr<TEScene> scene);
	void RenderDepthPass();
	void RenderMainPass(uint32_t imageIndex);
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
	VkPipeline _vkPipeline;

	VkSemaphore _imageAvailableSemaphore, _renderFinishedSemaphore;
	VkFence _inFlightFence;

	VkCommandPool _vkCommandPool;
	VkCommandBuffer _vkCommandBuffer;
};