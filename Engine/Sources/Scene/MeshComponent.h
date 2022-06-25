#pragma once

#include "CoreDefine.h"
#include "SceneComponent.h"

#include <glm/glm.hpp>

class TEMeshComponent : public TESceneComponent
{
public:
    TEMeshComponent();
    ~TEMeshComponent();

    void SetVertices(const std::vector<glm::vec3> &vertices);
    const std::vector<glm::vec3> &GetVertices();

private:
    std::vector<glm::vec3> _vertices;
};