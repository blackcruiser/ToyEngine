#pragma once

#include "CoreDefine.h"
#include "SceneComponent.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <functional>
 

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
    }
};

namespace std
{
    template <>
    struct hash<Vertex>
    {
        size_t operator()(Vertex const &vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}


class TEMeshComponent : public TESceneComponent
{
public:
    TEMeshComponent();
    ~TEMeshComponent();

    void SetVertices(const std::vector<Vertex> &vertices);
    const std::vector<Vertex> &GetVertices();

    void SetIndexes(const std::vector<uint32_t> &indexes);
    const std::vector<uint32_t> &GetIndexes();

private:
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indexes;
};