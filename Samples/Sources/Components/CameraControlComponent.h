#include "Scene\ScriptComponent.h"
#include "Input\InputSystem.h"

class CameraControlComponent : public TEScriptComponent
{
public:
    virtual void OnAttached() override;
    virtual void OnDetached() override;

    void OnMouseMove(double xpos, double ypos);

private:
    size_t _mouseMoveActionKey;
};