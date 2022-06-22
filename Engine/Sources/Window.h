#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

class TEWindow
{
public:
    TEWindow(const std::string &appName, int width, int height);
    ~TEWindow();

    bool ShouldClose();

    GLFWwindow *GetRawWindow();
    glm::ivec2 GetFramebufferSize();

private:
    GLFWwindow *_glfwWindow;
    int _width, _height;
};
