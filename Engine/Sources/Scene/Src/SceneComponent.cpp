#include "SceneComponent.h"

TESceneComponent::TESceneComponent(const EComponentType type) : _type(type)
{
}

TESceneComponent::~TESceneComponent()
{
}

const EComponentType TESceneComponent::GetType() const
{
    return _type;
}