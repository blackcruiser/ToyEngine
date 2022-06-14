#pragma once

#include <vulkan/vulkan.h>

#include <string>

enum class EShaderType
{
    Vertex = 0,
    Fragment
};

class TEShader
{
public:
    TEShader();

private:
    VkShaderModule _shader;

    std::string _path;

};