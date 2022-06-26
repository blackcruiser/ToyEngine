#include "TransformComponent.h"

TETransformComponent::TETransformComponent(): TESceneComponent(EComponentType::Transform), _transform(1)
{
}

TETransformComponent::~TETransformComponent()
{
}

void TETransformComponent::SetTransform(const glm::mat4x4 &transform)
{
    _transform = transform;
}

const glm::mat4x4 &TETransformComponent::GetTransform()
{
    return _transform;
}