#pragma once

#include "CoreDefine.h"
#include "SceneComponent.h"

#include <glm/glm.hpp>

class TETransformComponent : public TESceneComponent
{
public:
    TETransformComponent();
    ~TETransformComponent();

    void SetTransform(const glm::mat4x4 &transform);
    const glm::mat4x4 &GetTransform();

private:
    glm::mat4x4 _transform;
};