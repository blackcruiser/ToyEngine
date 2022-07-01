#include "Scene.h"
#include "SceneObject.h"
#include "CameraComponent.h"
#include "ScriptComponent.h"

#include <stdexcept>

TEScene::TEScene()
{
}

void TEScene::AddObject(TEPtr<TESceneObject> object)
{
    TEPtrArr<TEScriptComponent> scriptComponents = object->GetComponents<TEScriptComponent>();
    for (const TEPtr<TEScriptComponent> &scriptComponent : scriptComponents)
        scriptComponent->OnAttached();

    _objects.push_back(object);
}

void TEScene::RemoveObject(TEPtr<TESceneObject> object)
{
    _objects.erase(std::remove(_objects.begin(), _objects.end(), object));

    TEPtrArr<TEScriptComponent> scriptComponents = object->GetComponents<TEScriptComponent>();
    for (const TEPtr<TEScriptComponent> &scriptComponent : scriptComponents)
        scriptComponent->OnDetached();
}

const TEPtrArr<TESceneObject> &TEScene::GetObjects()
{
    return _objects;
}

void TEScene::SetCamera(TEPtr<TECameraComponent> cameraComponent)
{
    if (cameraComponent == nullptr || cameraComponent->GetObject().expired())
        throw std::runtime_error("cameraComponent is null or object is null");

    TEWeakPtr<TESceneObject> object = cameraComponent->GetObject();
    if (_cameraComponent != nullptr)
        RemoveObject(object.lock());
    AddObject(object.lock());

    _cameraComponent = cameraComponent;
}

TEPtr<TECameraComponent> TEScene::GetCamera()
{
    return _cameraComponent;
}