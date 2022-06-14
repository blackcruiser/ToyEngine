#pragma once

#include <memory>

class TEMaterial;

class TEObject
{
public:
    std::shared_ptr<TEMaterial> _material;
    // std::vector<glm::vec3> vertices;
};
