#include "TransformComponent.h"

TransformComponent::TransformComponent(): TESceneComponent(EComponentType::Transform)
{
}

TransformComponent::~TransformComponent()
{
}

void TransformComponent::SetTransform(const glm::mat4x4 &transform)
{
    _transform = transform;
}