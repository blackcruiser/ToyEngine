#pragma once

#include "CoreDefine.h"
#include "SceneComponent.h"

#include <string>
#include <optional>

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

    std::optional<std::reference_wrapper<std::string>> GetShaderPath(const EMaterialShaderType &type);

private:
    // EBlendType _blendType;
    std::map<EMaterialShaderType, std::string> _shaderMap;
};