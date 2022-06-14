#include "Scene.h"
#include "Object.h"
#include "Material.h"

std::shared_ptr<TEScene> TEScene::CreateSampleScene()
{
    std::shared_ptr<TEObject> object = std::make_shared<TEObject>();
    object->_material = std::make_shared<TEMaterial>();

    std::shared_ptr<TEScene> scene = std::make_shared<TEScene>();
    scene->AddObject(object);

    return scene;
}

TEScene::TEScene()
{
}

void TEScene::AddObject(std::shared_ptr<TEObject> object)
{
    _objects.push_back(object);
}

const std::vector<std::shared_ptr<TEObject>> &TEScene::GetObjects()
{
    return _objects;
}