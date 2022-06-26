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

void TESceneComponent::SetObject(TEWeakPtr<TESceneObject> object)
{
    _sceneObject = object;
}

TEWeakPtr<TESceneObject> TESceneComponent::GetObject()
{
    return _sceneObject;
}