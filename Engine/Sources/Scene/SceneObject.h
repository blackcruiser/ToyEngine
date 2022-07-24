#pragma once

#include "CoreDefine.h"
#include "SceneComponent.h"

#include <glm/glm.hpp>

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
        _componentArr.push_back(component);

        component->SetObject(shared_from_this());
    }

    template <typename T>
    requires std::is_convertible<T, TESceneComponent>::value
    TEPtr<T> GetComponent()
    {
        for (const TEPtr<TESceneComponent>& component : _componentArr)
        {
            TEPtr<T> result = std::dynamic_pointer_cast<T>(component);
            if (result != nullptr)
                return result;
        }

        return nullptr;
    }

    template <typename T>
    requires std::is_convertible<T, TESceneComponent>::value
    TEPtrArr<T> GetComponents()
    {
        TEPtrArr<T> components;

        for (const TEPtr<TESceneComponent> &component : _componentArr)
        {
            TEPtr<T> result =  std::dynamic_pointer_cast<T>(component);
            if (result != nullptr)
                components.push_back(result);
        }

        return components;
    }

private:
    TEPtrArr<TESceneComponent> _componentArr;
};
