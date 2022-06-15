#pragma once

#include <memory>

class TEMaterial;

class TEObject
{
public:
    TEPtr<TEMaterial> _material;
    // std::vector<glm::vec3> vertices;
};
