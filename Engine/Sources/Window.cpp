#include "Window.h"

#include <glfw/glfw3.h>

#include <stdexcept>

TEWindow::TEWindow() : _glfwWindow(nullptr)
{
}

void TEWindow::Init(const std::string &appName, int width, int height)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _glfwWindow = glfwCreateWindow(width, height, appName.c_str(), nullptr, nullptr);

    if (_glfwWindow == nullptr)
    {
        throw std::runtime_error("failed to create window!");
        return;
    }
}

void TEWindow::Cleanup()
{
    glfwDestroyWindow(_glfwWindow);
}

bool TEWindow::ShouldClose()
{
    return glfwWindowShouldClose(_glfwWindow);
}

GLFWwindow *TEWindow::GetRawWindow()
{
    return _glfwWindow;
}

glm::ivec2 TEWindow::GetFramebufferSize()
{
    glm::ivec2 size;
    glfwGetFramebufferSize(_glfwWindow, &size.x, &size.y);

    return size;
}