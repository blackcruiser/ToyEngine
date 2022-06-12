#include "Window.h"

#include <stdexcept>

TEWindow::TEWindow(VkInstance vkInstance) : _vkInstance(vkInstance),
                                            glfwWindow(nullptr), vkSurface(VK_NULL_HANDLE)
{
}

void TEWindow::Init(const std::string &appName, int width, int height)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindow = glfwCreateWindow(width, height, appName.c_str(), nullptr, nullptr);

    if (glfwWindow == nullptr)
    {
        throw std::runtime_error("failed to create window!");
        return;
    }

    if (glfwCreateWindowSurface(_vkInstance, glfwWindow, nullptr, &vkSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void TEWindow::Cleanup()
{
    vkDestroySurfaceKHR(_vkInstance, vkSurface, nullptr);

    glfwDestroyWindow(glfwWindow);
}