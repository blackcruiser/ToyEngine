#pragma once

#include "CoreDefine.h"

enum class EComponentType : int
{
    Invalid = 0,
    Transform = 1,
    Mesh = 2,
    Material = 3,
};

class SceneObject;

class TESceneComponent
{
public:
    TESceneComponent(const EComponentType type);
    ~TESceneComponent();

    const EComponentType GetType() const;

private:
    EComponentType _type;
    TEWeakPtr<SceneObject> _scencObject;
};