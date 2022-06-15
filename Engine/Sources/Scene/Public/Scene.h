#pragma once

#include "CommonDefine.h"

class TEObject;

class TEScene
{
public:
    static TEPtr<TEScene> CreateSampleScene();

public:
    TEScene();

    void AddObject(TEPtr<TEObject> object);

    const TEPtrArr<TEObject> &GetObjects();

private:
    TEPtrArr<TEObject> _objects;
};