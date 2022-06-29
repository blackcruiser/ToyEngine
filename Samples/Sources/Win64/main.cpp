#include "Application.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"
#include "Scene/TransformComponent.h"
#include "Scene/MeshComponent.h"
#include "Scene/MaterialComponent.h"
#include "Scene/CameraComponent.h"

TEPtr<TEScene> CreateSampleScene()
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

int main()
{
    TEPtr<TEScene> scene = CreateSampleScene();

    TEApplication app;
    app.Run(scene);

    return 0;
}