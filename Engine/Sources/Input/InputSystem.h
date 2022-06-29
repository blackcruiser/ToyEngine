#pragma once

#include "CoreDefine.h"

#include <GLFW/glfw3.h>

typedef void (*InputFunc)(double xpos, double ypos);

class TEInputSystem
{
public:
    static const TEInputSystem &GetInstance();

private:
    TEInputSystem();
    ~TEInputSystem();

public:
    void RegisterNotify(InputFunc func);
    void UnregisterNotify(InputFunc func);

    static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

private:
    std::vector<InputFunc> _inputFuncs;
};