#pragma once

#include <string>

enum class EShaderType
{
    Vertex = 0,
    Fragment
};

class TEShader
{
public:
    TEShader(const std::string &path);

    const std::string &GetPath();

private:
    std::string _path;
};