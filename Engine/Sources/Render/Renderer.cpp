#include "Renderer.h"
#include "Scene.h"
#include "Object.h"
#include "Material.h"

#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <array>
#include <cstddef>

static std::vector<char> readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VkShaderModule TEForwardRenderer::CreateShaderModule(const std::vector<char> &code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device->vkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    _vkShaderModules.push_back(shaderModule);

    return shaderModule;
}

void TEForwardRenderer::Init(TEPtr<TEDevice> device, TEPtr<TEWindow> window)
{
    _device = device;
    _window = window;

    SelectSurfaceFormat();

    CreateRenderPass();
    CreateSwapchain(_vkRenderPass);

    CreateCommandPool();
    CreateCommandBuffer();

    CreateSemaphore();
}

void TEForwardRenderer::SelectSurfaceFormat()
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_device->vkPhysicalDevice, _window->vkSurface, &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    if (formatCount != 0)
    {
        surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(_device->vkPhysicalDevice, _window->vkSurface, &formatCount, surfaceFormats.data());
    }

    for (size_t i = 0; i < surfaceFormats.size(); i++)
    {
        const VkSurfaceFormatKHR &currentSurfaceFormat = surfaceFormats[i];

        if (currentSurfaceFormat.colorSpace == VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && currentSurfaceFormat.format == VkFormat::VK_FORMAT_B8G8R8A8_SRGB)
        {
            _vkSurfaceFormat = currentSurfaceFormat;
            break;
        }
    }
}

void TEForwardRenderer::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _vkSurfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(_device->vkDevice, &renderPassInfo, nullptr, &_vkRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

VkPipeline TEForwardRenderer::CreatePipeline(TEPtr<TEMaterial> material)
{
    auto vertShaderCode = readFile("Shaders/VertexShader.spv");
    auto fragShaderCode = readFile("Shaders/FragmentShader.spv");

    // Shader
    VkShaderModule vkVerterShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule vkFragmentShaderModule = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vkVertexShaderStageCreateInfo{};
    vkVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkVertexShaderStageCreateInfo.pName = "main";
    vkVertexShaderStageCreateInfo.module = vkVerterShaderModule;

    VkPipelineShaderStageCreateInfo vkFragmentShaderStageCreateInfo{};
    vkFragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkFragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkFragmentShaderStageCreateInfo.pName = "main";
    vkFragmentShaderStageCreateInfo.module = vkFragmentShaderModule;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vkVertexShaderStageCreateInfo, vkFragmentShaderStageCreateInfo};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(glm::vec3);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    // attributeDescriptions[1].binding = 0;
    // attributeDescriptions[1].location = 1;
    // attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    // attributeDescriptions[1].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_vkExtent.width;
    viewport.height = (float)_vkExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _vkExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(_device->vkDevice, &pipelineLayoutInfo, nullptr, &_vkPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = _vkPipelineLayout;
    pipelineInfo.renderPass = _vkRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional

    VkPipeline vkPipeline;
    if (vkCreateGraphicsPipelines(_device->vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    return vkPipeline;
}

void TEForwardRenderer::CreateSemaphore()
{

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(_device->vkDevice, &semaphoreInfo, nullptr, &_imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(_device->vkDevice, &semaphoreInfo, nullptr, &_renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(_device->vkDevice, &fenceInfo, nullptr, &_inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create semaphores!");
    }
}

void TEForwardRenderer::CreateSwapchain(VkRenderPass renderPass)
{
    // Swap Chain

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_device->vkPhysicalDevice, _window->vkSurface, &presentModeCount, nullptr);

    std::vector<VkPresentModeKHR> presentModes;
    if (presentModeCount != 0)
    {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(_device->vkPhysicalDevice, _window->vkSurface, &presentModeCount, presentModes.data());
    }

    VkPresentModeKHR presentMode{};
    for (size_t i = 0; i < presentModes.size(); i++)
    {
        const VkPresentModeKHR &currentPresentMode = presentModes[i];

        if (currentPresentMode == VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = currentPresentMode;
            break;
        }
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->vkPhysicalDevice, _window->vkSurface, &capabilities);

    // GLFW uses two units when measuring sizes: pixels and screen coordinates.

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        _vkExtent = capabilities.currentExtent;
    }
    else
    {

        // glfwGetFramebufferSize: resolution of the window in pixel
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(_window->glfwWindow, &framebufferWidth, &framebufferHeight);

        _vkExtent.width = std::clamp(static_cast<uint32_t>(framebufferWidth), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        _vkExtent.height = std::clamp(static_cast<uint32_t>(framebufferHeight), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    VkSwapchainCreateInfoKHR vkSwapchainCreateInfo{};
    vkSwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfo.surface = _window->vkSurface;
    vkSwapchainCreateInfo.minImageCount = capabilities.minImageCount + 1;
    vkSwapchainCreateInfo.imageFormat = _vkSurfaceFormat.format;
    vkSwapchainCreateInfo.imageColorSpace = _vkSurfaceFormat.colorSpace;
    vkSwapchainCreateInfo.imageExtent = _vkExtent;
    vkSwapchainCreateInfo.imageArrayLayers = 1;
    vkSwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (_device->graphicQueueFamilyIndex != _device->presentQueueFamilyIndex)
    {
        uint32_t queueFamilyIndexs[] = {_device->graphicQueueFamilyIndex, _device->presentQueueFamilyIndex};
        vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        vkSwapchainCreateInfo.queueFamilyIndexCount = 2;
        vkSwapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndexs;
    }
    else
    {
        vkSwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkSwapchainCreateInfo.queueFamilyIndexCount = 0;     // Optional
        vkSwapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    vkSwapchainCreateInfo.preTransform = capabilities.currentTransform;
    vkSwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfo.presentMode = presentMode;
    vkSwapchainCreateInfo.clipped = VK_TRUE;
    vkSwapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_device->vkDevice, &vkSwapchainCreateInfo, nullptr, &_vkSwapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(_device->vkDevice, _vkSwapchain, &imageCount, nullptr);
    _vkImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device->vkDevice, _vkSwapchain, &imageCount, _vkImages.data());

    // Image View
    _vkImageViews.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = _vkImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = _vkSurfaceFormat.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_device->vkDevice, &imageViewCreateInfo, nullptr, &_vkImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }

    _vkFramebuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        VkImageView attachments[] = {
            _vkImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = _vkExtent.width;
        framebufferInfo.height = _vkExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_device->vkDevice, &framebufferInfo, nullptr, &_vkFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void TEForwardRenderer::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = _device->graphicQueueFamilyIndex;

    if (vkCreateCommandPool(_device->vkDevice, &poolInfo, nullptr, &_vkCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void TEForwardRenderer::CreateCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _vkCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(_device->vkDevice, &allocInfo, &_vkCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void TEForwardRenderer::GatherObjects(TEPtr<TEScene> scene)
{
    const TEPtrArr<TEObject> &objects = scene->GetObjects();

    for (auto &object : objects)
    {
        TEPtr<TEMaterial> material = object->_material;
        std::uintptr_t address = reinterpret_cast<std::uintptr_t>(material.get());
        if (_objectsToRender.find(address) == _objectsToRender.end())
            _objectsToRender.emplace(address, TEPtrArr<TEObject>());
        TEPtrArr<TEObject> &objectArr = _objectsToRender.at(address);
        objectArr.push_back(object);
    }
}

uint32_t TEForwardRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_device->vkPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void TEForwardRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_device->vkDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_device->vkDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_device->vkDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(_device->vkDevice, buffer, bufferMemory, 0);
}

void TEForwardRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _vkCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device->vkDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(_device->vkGraphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_device->vkGraphicQueue);

    vkFreeCommandBuffers(_device->vkDevice, _vkCommandPool, 1, &commandBuffer);
}

void TEForwardRenderer::RenderFrame(TEPtr<TEScene> scene)
{
    GatherObjects(scene);

    vkWaitForFences(_device->vkDevice, 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(_device->vkDevice, 1, &_inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(_device->vkDevice, _vkSwapchain, UINT64_MAX, _imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _vkRenderPass;
    renderPassInfo.framebuffer = _vkFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _vkExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    if (vkBeginCommandBuffer(_vkCommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    vkCmdBeginRenderPass(_vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (auto &pair : _objectsToRender)
    {
        auto &objectArr = pair.second;
        if (objectArr.empty())
            continue;

        TEPtr<TEMaterial> material = objectArr[0]->_material;

        VkPipeline vkPipeline;

        std::uintptr_t address = reinterpret_cast<std::uintptr_t>(material.get());
        if (_pipelines.find(address) == _pipelines.end())
        {
            vkPipeline = CreatePipeline(material);
            _pipelines.insert(std::make_pair(address, vkPipeline));
        }
        else
        {
            vkPipeline = _pipelines[address];
        }

        size_t totalBufferSize = 0;
        for (auto &object : objectArr)
        {
            totalBufferSize = object->vertices.size() * sizeof(glm::vec3);
        }

        if (_stagingBuffer != VK_NULL_HANDLE && totalBufferSize > _stagingBufferSize)
        {
            vkFreeMemory(_device->vkDevice, _stagingBufferMemory, nullptr);
            vkDestroyBuffer(_device->vkDevice, _stagingBuffer, nullptr);
            _stagingBuffer = VK_NULL_HANDLE;
        }

        if (_stagingBuffer == VK_NULL_HANDLE)
        {
            createBuffer(totalBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _stagingBuffer, _stagingBufferMemory);
        }

        if (_vertexBuffer != VK_NULL_HANDLE && totalBufferSize > _vertexBufferSize)
        {
            vkFreeMemory(_device->vkDevice, _vertexBufferMemory, nullptr);
            vkDestroyBuffer(_device->vkDevice, _vertexBuffer, nullptr);
            _vertexBuffer = VK_NULL_HANDLE;
        }

        if (_vertexBuffer == VK_NULL_HANDLE)
        {
            createBuffer(totalBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, _vertexBufferMemory);
        }

        void *data;
        vkMapMemory(_device->vkDevice, _stagingBufferMemory, 0, totalBufferSize, 0, &data);

        uint8_t *dataPtr = reinterpret_cast<uint8_t *>(data);
        uint32_t vertexCounts = 0;
        for (auto &object : objectArr)
        {
            size_t bufferSize = object->vertices.size() * sizeof(glm::vec3);
            memcpy(dataPtr, object->vertices.data(), bufferSize);
            dataPtr = dataPtr + bufferSize;
            vertexCounts += object->vertices.size();
        }

        vkUnmapMemory(_device->vkDevice, _stagingBufferMemory);

        copyBuffer(_stagingBuffer, _vertexBuffer, totalBufferSize);

        vkCmdBindPipeline(_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

        VkBuffer vertexBuffers[] = {_vertexBuffer};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdDraw(_vkCommandBuffer, vertexCounts, 1, 0, 0);
    }

    vkCmdEndRenderPass(_vkCommandBuffer);

    if (vkEndCommandBuffer(_vkCommandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphore};
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_vkCommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_device->vkGraphicQueue, 1, &submitInfo, _inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapchains[] = {_vkSwapchain};
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(_device->vkPresentQueue, &presentInfo);
}

void TEForwardRenderer::Cleanup()
{

    vkFreeMemory(_device->vkDevice, _vertexBufferMemory, nullptr);
    vkDestroyBuffer(_device->vkDevice, _vertexBuffer, nullptr);

    vkFreeMemory(_device->vkDevice, _stagingBufferMemory, nullptr);
    vkDestroyBuffer(_device->vkDevice, _stagingBuffer, nullptr);

    vkFreeCommandBuffers(_device->vkDevice, _vkCommandPool, 1, &_vkCommandBuffer);
    vkDestroyCommandPool(_device->vkDevice, _vkCommandPool, nullptr);

    vkDestroySemaphore(_device->vkDevice, _imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(_device->vkDevice, _renderFinishedSemaphore, nullptr);
    vkDestroyFence(_device->vkDevice, _inFlightFence, nullptr);

    vkDestroyPipelineLayout(_device->vkDevice, _vkPipelineLayout, nullptr);

    for (auto iter = _pipelines.begin(); iter != _pipelines.end(); iter++)
        vkDestroyPipeline(_device->vkDevice, iter->second, nullptr);

    vkDestroyRenderPass(_device->vkDevice, _vkRenderPass, nullptr);

    for (VkShaderModule &shaderModule : _vkShaderModules)
    {
        vkDestroyShaderModule(_device->vkDevice, shaderModule, nullptr);
    }

    for (auto imageView : _vkImageViews)
    {
        vkDestroyImageView(_device->vkDevice, imageView, nullptr);
    }

    for (VkFramebuffer &framebuffer : _vkFramebuffers)
    {
        vkDestroyFramebuffer(_device->vkDevice, framebuffer, nullptr);
    }

    vkDestroySwapchainKHR(_device->vkDevice, _vkSwapchain, nullptr);
}
