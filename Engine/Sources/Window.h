#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

class TEWindow
{
public:
    TEWindow();

    void Init(const std::string &appName, int width, int height);
    void Cleanup();

    bool ShouldClose();

    GLFWwindow *GetRawWindow();
    glm::ivec2 GetFramebufferSize();

private:
    GLFWwindow *_glfwWindow;
};
