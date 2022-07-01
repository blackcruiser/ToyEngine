#include "CameraControlComponent.h"

#include <functional>
#include <format>
#include <iostream>

void CameraControlComponent::OnAttached()
{
    InputAction action = std::bind(&CameraControlComponent::OnMouseMove, this, std::placeholders::_1, std::placeholders::_2);
    _mouseMoveActionKey = TEInputSystem::GetInstance().RegisterAction(action);
}

void CameraControlComponent::OnDetached()
{
    TEInputSystem::GetInstance().UnregisterAction(_mouseMoveActionKey);
}

void CameraControlComponent::OnMouseMove(double xpos, double ypos)
{
    std::cout << std::format("camera {0}, {1}", xpos, ypos) << std::endl;
}