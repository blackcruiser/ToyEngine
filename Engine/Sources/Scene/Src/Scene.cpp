#include "Scene.h"
#include "SceneObject.h"
#include "TransformComponent.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "CameraComponent.h"

#include <stdexcept>

TEPtr<TEScene> TEScene::CreateSampleScene()
{
    TEPtr<TEScene> scene = std::make_shared<TEScene>();

    TEPtr<TESceneObject> meshObject = std::make_shared<TESceneObject>();

    TEPtr<TETransformComponent> transform = std::make_shared<TETransformComponent>();

    TEPtr<TEMeshComponent> mesh = std::make_shared<TEMeshComponent>();

    meshObject->AddComponent(transform);
    meshObject->AddComponent(mesh);

    std::vector<glm::vec3> vertices =
        {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}};

    mesh->SetVertices(vertices);

    std::vector<int> indexes =
        {0, 1, 2, 0, 2, 3, 0, 1, 5, 0, 5, 4, 1, 2, 6, 1, 6, 5, 2, 3, 6, 3, 7, 6, 0, 3, 4, 3, 7, 4, 5, 6, 7, 5, 7, 4};

    mesh->SetIndexes(indexes);

    scene->AddObject(meshObject);

    TEPtr<TESceneObject> cameraObject = std::make_shared<TESceneObject>();
    TEPtr<TECameraComponent> camera = std::make_shared<TECameraComponent>();
    cameraObject->AddComponent(camera);

    scene->SetCamera(camera);

    return scene;
}

TEScene::TEScene()
{
}

void TEScene::AddObject(TEPtr<TESceneObject> object)
{
    _objects.push_back(object);
}

void TEScene::RemoveObject(TEPtr<TESceneObject> object)
{
    _objects.erase(std::remove(_objects.begin(), _objects.end(), object));
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