#include "MaterialComponent.h"

TEMaterialComponent::TEMaterialComponent() : TESceneComponent(EComponentType::Material)
{
}

std::optional<std::reference_wrapper<std::string>> TEMaterialComponent::GetShaderPath(const EMaterialShaderType &type)
{
    if (_shaderMap.find(type) == _shaderMap.end())
        return std::nullopt;
    else
        return _shaderMap[type];
}