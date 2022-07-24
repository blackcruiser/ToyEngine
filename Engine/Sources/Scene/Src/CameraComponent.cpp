#include "CameraComponent.h"
#include "TransformComponent.h"
#include "SceneObject.h"

#include <glm/ext.hpp>

TECameraComponent::TECameraComponent() : TESceneComponent(EComponentType::Camera), _projectMatrix(glm::identity<glm::mat4x4>())
{
}

TECameraComponent::~TECameraComponent()
{
}

glm::mat4x4 TECameraComponent::GetViewMatrix()
{
	glm::mat4x4 viewMatrix{ glm::identity<glm::mat4x4>() };

	TEPtr<TETransformComponent> transformComponent = GetObject() ? GetObject()->GetComponent<TETransformComponent>() : nullptr;
	if (transformComponent != nullptr)
	{
		viewMatrix = glm::inverse(transformComponent->GetTransform());
	}

	return viewMatrix;
}

void TECameraComponent::SetProjectMatrix(const glm::mat4x4& matrix)
{
	_projectMatrix = matrix;
}

const glm::mat4x4& TECameraComponent::GetProjectMatrix()
{
	return _projectMatrix;
}