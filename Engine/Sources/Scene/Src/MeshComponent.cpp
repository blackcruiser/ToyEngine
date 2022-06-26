#include "MeshComponent.h"



TEMeshComponent::TEMeshComponent(): TESceneComponent(EComponentType::Mesh)
{
}

TEMeshComponent::~TEMeshComponent()
{
}

void TEMeshComponent::SetVertices(const std::vector<glm::vec3> &vertices)
{
    _vertices = vertices;
}

const std::vector<glm::vec3> &TEMeshComponent::GetVertices()
{
    return _vertices;
}

void TEMeshComponent::SetIndexes(const std::vector<int> &indexes)
{
    _indexes = indexes;
}

const std::vector<int> &TEMeshComponent::GetIndexes()
{
    return _indexes;
}