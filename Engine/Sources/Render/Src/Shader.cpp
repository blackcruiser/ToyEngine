#include "Shader.h"

TEShader::TEShader(const std::string &path) : _path(path)
{
}

const std::string &TEShader::GetPath()
{
    return _path;
}