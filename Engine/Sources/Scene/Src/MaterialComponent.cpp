#include "MaterialComponent.h"

TEMaterialComponent::TEMaterialComponent() : TESceneComponent(EComponentType::Material)
{
}

TEMaterialComponent::~TEMaterialComponent()
{
}

void TEMaterialComponent::SetShader(const EMaterialShaderType& type, const std::string& shaderPath)
{
    if (_shaderMap.find(type) == _shaderMap.end())
        _shaderMap.insert(std::make_pair(type, TEMaterialShaderInfo()));

    TEMaterialShaderInfo& shaderInfo = _shaderMap[type];
    shaderInfo.filePath = shaderPath;
}

std::optional<std::reference_wrapper<TEMaterialShaderInfo>> TEMaterialComponent::GetShaderInfo(const EMaterialShaderType &type)
{
    if (_shaderMap.find(type) == _shaderMap.end())
        return std::nullopt;
    else
        return _shaderMap[type];
}