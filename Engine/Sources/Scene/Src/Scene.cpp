#include "Scene.h"
#include "SceneObject.h"
#include "TransformComponent.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"

TEPtr<TEScene> TEScene::CreateSampleScene()
{
    TEPtr<TESceneObject> object = std::make_shared<TESceneObject>();
    TEPtr<TransformComponent> transform = std::make_shared<TransformComponent>();
    TEPtr<TEMeshComponent> mesh = std::make_shared<TEMeshComponent>();

    object->AddComponent(transform);
    object->AddComponent(mesh);

    std::vector<glm::vec3> vertices =
        {{1.0f, 0.0f, 0.0f},
         {0.0f, 1.0f, 0.0f},
         {0.0f, 0.0f, 0.0f}};
    mesh->SetVertices(vertices);

    TEPtr<TEScene> scene = std::make_shared<TEScene>();
    scene->AddObject(object);

    return scene;
}

TEScene::TEScene()
{
}

void TEScene::AddObject(TEPtr<TESceneObject> object)
{
    _objects.push_back(object);
}

const TEPtrArr<TESceneObject> &TEScene::GetObjects()
{
    return _objects;
}