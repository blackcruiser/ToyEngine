#include "CoreDefine.h"
#include "Application.h"
#include "Scene/Scene.h"
#include "Scene/SceneObject.h"
#include "Scene/TransformComponent.h"
#include "Scene/MeshComponent.h"
#include "Scene/MaterialComponent.h"
#include "Scene/CameraComponent.h"
#include "Components/CameraControlComponent.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

TEPtr<TEScene> CreateSampleScene()
{
    TEPtr<TEScene> scene = std::make_shared<TEScene>();


    TEPtr<TESceneObject> meshObject = std::make_shared<TESceneObject>();
    TEPtr<TETransformComponent> meshTransformComponent = std::make_shared<TETransformComponent>();
    meshObject->AddComponent(meshTransformComponent);

    TEPtr<TEMeshComponent> meshComponent = std::make_shared<TEMeshComponent>();
    std::vector<glm::vec3> vertices =
        {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}};
    std::vector<int> indexes =
    { 0, 1, 2, 0, 2, 3, 0, 5, 1, 0, 4, 5, 1, 6, 2, 1, 5, 6, 2, 6, 3, 3, 6, 7, 0, 3, 4, 3, 7, 4, 5, 7, 6, 5, 4, 7 };
    meshComponent->SetVertices(vertices);
    meshComponent->SetIndexes(indexes);
    meshObject->AddComponent(meshComponent);

    TEPtr<TEMaterialComponent> meshMaterialComponent = std::make_shared<TEMaterialComponent>();
    meshMaterialComponent->SetShader(EMaterialShaderType::Vertex, "LocalToClipSpaceVertexShader.glsl");
    meshMaterialComponent->SetShader(EMaterialShaderType::Fragment, "LambertBlinnPhoneFragmentShader.glsl");
    meshObject->AddComponent(meshMaterialComponent);

    scene->AddObject(meshObject);


    TEPtr<TESceneObject> cameraObject = std::make_shared<TESceneObject>();

    TEPtr<TETransformComponent> cameraTransformComponent = std::make_shared<TETransformComponent>();
    glm::mat4x4 cameraTransform = glm::translate(glm::identity<glm::mat4x4>(), glm::vec3(0.0f, 0.0f, 10.0f));
    cameraTransformComponent->SetTransform(cameraTransform);
    cameraObject->AddComponent(cameraTransformComponent);

    TEPtr<TECameraComponent> cameraComponent = std::make_shared<TECameraComponent>();
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    cameraComponent->SetProjectMatrix(projectionMatrix);
    cameraObject->AddComponent(cameraComponent);

    TEPtr<CameraControlComponent> cameraControlComponent = std::make_shared<CameraControlComponent>();
    cameraObject->AddComponent(cameraControlComponent);

    scene->SetCamera(cameraComponent);

    return scene;
}

int main()
{
    TEPtr<TEScene> scene = CreateSampleScene();

    TEApplication app;
    app.Run(scene);

    return 0;
}