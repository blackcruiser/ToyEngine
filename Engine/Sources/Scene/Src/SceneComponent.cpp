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

void TESceneComponent::SetObject(TEPtr<TESceneObject> object)
{
    _sceneObject = object;
}

TEPtr<TESceneObject> TESceneComponent::GetObject()
{
    return _sceneObject.lock();
}