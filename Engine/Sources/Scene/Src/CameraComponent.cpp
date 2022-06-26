#include "CameraComponent.h"

#include <glm/ext.hpp>

TECameraComponent::TECameraComponent() : TESceneComponent(EComponentType::Camera), _viewMatrix(glm::identity<glm::mat4x4>()), _projectMatrix(1)
{
}

TECameraComponent::~TECameraComponent()
{
}

void TECameraComponent::SetViewMatrix(const glm::mat4x4 &matrix)
{
    _viewMatrix = matrix;
}

const glm::mat4x4 &TECameraComponent::GetViewMatrix()
{
    return _viewMatrix;
}

void TECameraComponent::SetProjectMatrix(const glm::mat4x4 &matrix)
{
    _projectMatrix = matrix;
}

const glm::mat4x4 &TECameraComponent::GetProjectMatrix()
{
    return _projectMatrix;
}