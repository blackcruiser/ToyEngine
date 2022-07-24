#include "Scene\ScriptComponent.h"
#include "Input\InputSystem.h"

#include <glm/glm.hpp>

#include <set>

class CameraControlComponent : public TEScriptComponent
{
public:
    CameraControlComponent();
    ~CameraControlComponent();

    virtual void OnAttached() override;
    virtual void OnDetached() override;

    void OnMouseInput(const glm::vec2& position);
    void OnKeyboardInput(int key, int action);

private:
    size_t _mouseActionKey, _keyboardActionKey;

    glm::vec2 cachedMousePosition;
    std::set<int> cachedPressedKey;
};