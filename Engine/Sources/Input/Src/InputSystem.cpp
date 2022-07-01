#include "InputSystem.h"

#include <iostream>
#include <format>

TEInputSystem &TEInputSystem::GetInstance()
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

size_t TEInputSystem::RegisterAction(InputAction action)
{
    _inputActions.push_back(action);
    return _inputActions.size() - 1;
}

void TEInputSystem::UnregisterAction(size_t key)
{
    _inputActions.erase(_inputActions.begin() + key);
}

void TEInputSystem::cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    for (const InputAction &action : GetInstance()._inputActions)
        action(xpos, ypos);
}