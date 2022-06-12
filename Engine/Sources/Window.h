#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <string>

class TEWindow
{
public:
    TEWindow(VkInstance vkInstance);

    void Init(const std::string &appName, int width, int height);
    void Cleanup();

public:
    GLFWwindow *glfwWindow;
    VkSurfaceKHR vkSurface;

private:
    VkInstance _vkInstance;
};
