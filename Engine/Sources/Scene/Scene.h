#pragma once

#include "CoreDefine.h"

class TESceneObject;
class TECameraComponent;

class TEScene
{
public:
    static TEPtr<TEScene> CreateSampleScene();

public:
    TEScene();

    void AddObject(TEPtr<TESceneObject> object);
    void RemoveObject(TEPtr<TESceneObject> object);
    const TEPtrArr<TESceneObject> &GetObjects();

    void SetCamera(TEPtr<TECameraComponent> cameraComponent);
    TEPtr<TECameraComponent> GetCamera();

private:
    TEPtrArr<TESceneObject> _objects;
    TEPtr<TECameraComponent> _cameraComponent;
};