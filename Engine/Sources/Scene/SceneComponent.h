#pragma once

#include "CoreDefine.h"

enum class EComponentType : int
{
    Invalid = 0,
    Camera,
    Transform,
    Mesh,
    Material,
    Script,
};

class TESceneObject;

class TESceneComponent
{
public:
    TESceneComponent(const EComponentType type);
    ~TESceneComponent();

    const EComponentType GetType() const;

    void SetObject(TEWeakPtr<TESceneObject> object);
    TEWeakPtr<TESceneObject> GetObject();

private:
    EComponentType _type;
    TEWeakPtr<TESceneObject> _sceneObject;
};