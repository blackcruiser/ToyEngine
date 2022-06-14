#pragma once

#include "Device.h"
#include "Window.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

class TEScene;

class TERendererInterface
{
public:
	virtual void Init(std::shared_ptr<TEDevice> device, std::shared_ptr<TEWindow> window) = 0;
	virtual void RenderFrame(std::shared_ptr<TEScene> scene) = 0;
	virtual void Cleanup() = 0;

private:
	
};


class TEForwardRenderer : public TERendererInterface
{
public:
	virtual void Init(std::shared_ptr<TEDevice> device, std::shared_ptr<TEWindow> window) override;
	void SelectSurfaceFormat();
	void CreateRenderPass();
	void CreateMainPipeline();
	void CreateSwapchain(VkRenderPass renderPass);
	
	void CreateCommandPool();
	void CreateCommandBuffer();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	void RenderDepthPass();
	void RenderMainPass(uint32_t imageIndex);
	virtual void RenderFrame(std::shared_ptr<TEScene> scene) override;


	virtual void Cleanup() override;

private:
	std::shared_ptr<TEDevice> _device;
	std::shared_ptr<TEWindow> _window;


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