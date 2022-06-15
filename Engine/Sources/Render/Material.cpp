#include "Material.h"

TEMaterial::TEMaterial()
{

}

TEPtr<TEShader> TEMaterial::GetShader(const EShaderType &type)
{
    if (_shaderMap.find(type) == _shaderMap.end())
        return nullptr;
    else
        return _shaderMap[type];
}