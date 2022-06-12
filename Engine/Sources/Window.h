#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>


class TEWindow
{
public:
    GLFWwindow* glfwWindow;
    VkSurfaceKHR vkSurface;
};
