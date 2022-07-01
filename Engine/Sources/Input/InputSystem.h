#pragma once

#include "CoreDefine.h"

#include <GLFW/glfw3.h>

#include <functional>

typedef std::function<void(double xpos, double ypos)> InputAction;

class TEInputSystem
{
public:
    static TEInputSystem &GetInstance();

private:
    TEInputSystem();
    ~TEInputSystem();

public:
    size_t RegisterAction(InputAction action);
    void UnregisterAction(size_t key);

    static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

private:
    std::vector<InputAction> _inputActions;
};