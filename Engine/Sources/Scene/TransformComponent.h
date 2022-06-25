#pragma once

#include "CoreDefine.h"
#include "SceneComponent.h"

#include <glm/glm.hpp>

class TransformComponent : public TESceneComponent
{
public:
    TransformComponent();
    ~TransformComponent();

    void SetTransform(const glm::mat4x4 &transform);

private:
    glm::mat4x4 _transform;
};