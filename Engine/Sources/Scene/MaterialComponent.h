#pragma once

#include "CoreDefine.h"
#include "SceneComponent.h"

#include <string>
#include <optional>
#include <filesystem>

enum class EMaterialShaderType : int
{
    Vertex,
    Fragment,
};


struct TEMaterialShaderInfo
{
    std::filesystem::path filePath;
    std::filesystem::path bytecodePath;
};


class TEMaterialComponent : public TESceneComponent
{
public:
    TEMaterialComponent();
    ~TEMaterialComponent();

    void SetShader(const EMaterialShaderType& type, const std::string& shaderPath);
    std::optional<std::reference_wrapper<TEMaterialShaderInfo>> GetShaderInfo(const EMaterialShaderType &type);

private:
    // EBlendType _blendType;
    std::map<EMaterialShaderType, TEMaterialShaderInfo> _shaderMap;
};