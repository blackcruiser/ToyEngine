#pragma once

#include "Shader.h"

#include <memory>
#include <map>

class TEMaterial
{
public:
    TEMaterial();

    std::shared_ptr<TEShader> GetShader(const EShaderType &type);

private:
    //EBlendType _blendType;
    std::map<EShaderType, std::shared_ptr<TEShader>> _shaderMap;

};