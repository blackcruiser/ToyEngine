#include "MeshComponent.h"



TEMeshComponent::TEMeshComponent(): TESceneComponent(EComponentType::Mesh)
{
}

TEMeshComponent::~TEMeshComponent()
{
}

void TEMeshComponent::SetVertices(const std::vector<Vertex> &vertices)
{
    _vertices = vertices;
}

const std::vector<Vertex> &TEMeshComponent::GetVertices()
{
    return _vertices;
}

void TEMeshComponent::SetIndexes(const std::vector<uint32_t> &indexes)
{
    _indexes = indexes;
}

const std::vector<uint32_t> &TEMeshComponent::GetIndexes()
{
    return _indexes;
}