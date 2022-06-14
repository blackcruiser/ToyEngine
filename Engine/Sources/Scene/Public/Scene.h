#pragma once

#include <memory>
#include <vector>

class TEObject;

class TEScene
{
public:
    static std::shared_ptr<TEScene> CreateSampleScene();

public:
    TEScene();

    void AddObject(std::shared_ptr<TEObject> object);

    const std::vector<std::shared_ptr<TEObject>> &GetObjects();

private:
    std::vector<std::shared_ptr<TEObject>> _objects;
};