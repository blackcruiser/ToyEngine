#pragma once

#include "CoreDefine.h"
#include "SceneComponent.h"

#include <glm/glm.hpp>

class TECameraComponent : public TESceneComponent
{
public:
    TECameraComponent();
    ~TECameraComponent();

    void SetViewMatrix(const glm::mat4x4 &matrix);
    const glm::mat4x4& GetViewMatrix();

    void SetProjectMatrix(const glm::mat4x4 &matrix);
    const glm::mat4x4& GetProjectMatrix();

private:
    glm::mat4x4 _viewMatrix, _projectMatrix;
};