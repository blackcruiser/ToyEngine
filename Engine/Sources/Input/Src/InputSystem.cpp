#include "InputSystem.h"

#include <iostream>
#include <format>

TEInputSystem& TEInputSystem::GetInstance()
{
    static TEInputSystem instance;

    return instance;
}

TEInputSystem::TEInputSystem()
{
}

TEInputSystem::~TEInputSystem()
{
}

size_t TEInputSystem::RegisterMouseAction(MouseAction action)
{
    _mouseActions.push_back(action);
    return _mouseActions.size() - 1;
}

void TEInputSystem::UnregisterMouseAction(size_t key)
{
    _mouseActions.erase(_mouseActions.begin() + key);
}

size_t TEInputSystem::RegisterKeyboardAction(KeyboardAction action)
{
    _keyboardActions.push_back(action);
    return _keyboardActions.size() - 1;
}

void TEInputSystem::UnregisterKeyboardAction(size_t key)
{
    _keyboardActions.erase(_keyboardActions.begin() + key);
}

void TEInputSystem::cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    glm::vec2 position{ xpos, ypos };
    for (const MouseAction& mouseAction : GetInstance()._mouseActions)
        mouseAction(position);
}

void TEInputSystem::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    for (const KeyboardAction& keyboardAction : GetInstance()._keyboardActions)
        keyboardAction(key, action);
}