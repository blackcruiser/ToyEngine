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

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

void LoadModel(const std::string &path, std::vector<Vertex> &vertices,
               std::vector<uint32_t> &indices)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]};

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

            if (attrib.normals.empty() == false)
            {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]};
            }
            else
                vertex.normal = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }
}

TEPtr<TEScene> CreateSampleScene()
{
    TEPtr<TEScene> scene = std::make_shared<TEScene>();

    TEPtr<TESceneObject> meshObject = std::make_shared<TESceneObject>();
    TEPtr<TETransformComponent> meshTransformComponent = std::make_shared<TETransformComponent>();
    meshObject->AddComponent(meshTransformComponent);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indexes;
    LoadModel("../Samples/Resources/Meshes/viking_room.obj", vertices, indexes);

    TEPtr<TEMeshComponent> meshComponent = std::make_shared<TEMeshComponent>();
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