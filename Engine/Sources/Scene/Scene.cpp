#include "Scene.h"
#include "Object.h"
#include "Material.h"

TEPtr<TEScene> TEScene::CreateSampleScene()
{
    TEPtr<TEObject> object = std::make_shared<TEObject>();
    object->vertices =
        {{1.0f, 0.0f, 0.0f},
         {0.0f, 1.0f, 0.0f},
         {0.0f, 0.0f, 0.0f}};
    object->_material = std::make_shared<TEMaterial>();

    TEPtr<TEScene> scene = std::make_shared<TEScene>();
    scene->AddObject(object);

    return scene;
}

TEScene::TEScene()
{
}

void TEScene::AddObject(TEPtr<TEObject> object)
{
    _objects.push_back(object);
}

const TEPtrArr<TEObject> &TEScene::GetObjects()
{
    return _objects;
}