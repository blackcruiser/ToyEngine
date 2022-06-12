#pragma once

#include <vulkan/vulkan.h>

class Shader
{
public:
    Shader();

private:
    VkShaderModule _shader;
}