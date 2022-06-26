#pragma once

#include "CoreDefine.h"
#include "SceneComponent.h"

#include <glm/glm.hpp>

#include <map>
#include <memory>

class TESceneObject : public std::enable_shared_from_this<TESceneObject>
{
public:
    TESceneObject();
    ~TESceneObject();

    template <typename T>
    requires std::is_convertible<T, TESceneComponent>::value
    void AddComponent(TEPtr<T> component)
    {
        size_t hash = typeid(T).hash_code();
        _componentMap.insert(std::make_pair(hash, component));

        component->SetObject(weak_from_this());
    }

    template <typename T>
    requires std::is_convertible<T, TESceneComponent>::value
    TEPtr<T> GetComponent()
    {
        size_t hash = typeid(T).hash_code();
        auto iter = _componentMap.find(hash);
        if (iter == _componentMap.end())
            return nullptr;

        TEPtr<TESceneComponent> component = iter->second;
        return std::static_pointer_cast<T>(component);
    }

private:
    std::multimap<size_t, TEPtr<TESceneComponent>> _componentMap;
};
