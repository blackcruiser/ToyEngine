#pragma once

#include "CommonDefine.h"

#include <glm/glm.hpp>

class TEMaterial;

class TEObject
{
public:
    TEPtr<TEMaterial> _material;
    std::vector<glm::vec3> vertices;
};
