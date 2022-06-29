#include "InputSystem.h"

#include <iostream>
#include <format>

const TEInputSystem &TEInputSystem::GetInstance()
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

void TEInputSystem::RegisterNotify(InputFunc func)
{
}

void TEInputSystem::UnregisterNotify(InputFunc func)
{
}

void TEInputSystem::cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    std::cout << std::format("{0}, {1}", xpos, ypos) << std::endl;
    for (const InputFunc &func : GetInstance()._inputFuncs)
        func(xpos, ypos);
}