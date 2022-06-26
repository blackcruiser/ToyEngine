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

    void SetIndexes(const std::vector<int> &vertices);
    const std::vector<int> &GetIndexes();

private:
    std::vector<glm::vec3> _vertices;
    std::vector<int> _indexes;
};