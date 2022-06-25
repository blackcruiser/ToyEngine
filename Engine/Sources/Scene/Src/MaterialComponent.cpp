#include "MaterialComponent.h"

TEMaterialComponent::TEMaterialComponent() : TESceneComponent(EComponentType::Material)
{
}

const std::string &TEMaterialComponent::GetShaderPath(const EMaterialShaderType &type)
{
    if (_shaderMap.find(type) == _shaderMap.end())
        return std::string();
    else
        return _shaderMap[type];
}