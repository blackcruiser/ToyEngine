#pragma once

#include "CoreDefine.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

class TEInputSystem;

class TEWindow
{
public:
    TEWindow(const std::string &appName, int width, int height);
    ~TEWindow();

    glm::ivec2 GetFramebufferSize();

    void RegisterInput(const TEInputSystem &inputSystem);
    void UnregisterInput(const TEInputSystem &inputSystem);

    bool ShouldClose();

    GLFWwindow *GetRawWindow();

private:
    GLFWwindow *_glfwWindow;
    int _width, _height;
};
