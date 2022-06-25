#pragma once

#include "CoreDefine.h"
#include "SceneComponent.h"

#include <string>

enum class EMaterialShaderType : int
{
    Vertex,
    Fragment,
};

class TEMaterialComponent : public TESceneComponent
{
public:
    TEMaterialComponent();
    ~TEMaterialComponent();

    const std::string &GetShaderPath(const EMaterialShaderType &type);

private:
    // EBlendType _blendType;
    std::map<EMaterialShaderType, std::string> _shaderMap;
};