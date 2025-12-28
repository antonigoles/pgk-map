#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Scene/GameObject.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Misc/Types.hpp>
#include <Engine/Support/FpsCamera.hpp>
#include <Engine/Core/Scene/SceneContext.hpp>
#include <iostream>

namespace GameApp
{
	void cameraControlScript(Engine::UpdateFunctionData data) {
		Engine::FpsCamera* camera = static_cast<Engine::FpsCamera*>(static_cast<Engine::Updateable*>(data.sourcePointer));
		auto window = data.sceneContext->glfwWindow;
		glm::vec3 translation = {0,0,0};
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			translation += camera->getForward();
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			translation -= camera->getForward();
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			translation -= camera->getRight();
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			translation += camera->getRight();
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			translation += glm::vec3{0, 1.0f, 0};
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			translation -= glm::vec3{0, 1.0f, 0};
		}

		translation = glm::length2(translation) == 0.0f ? translation : glm::normalize(translation) * 0.1f;

		camera->transform.setPosition(
			camera->transform.getPosition() + translation * data.deltaTime
		);
	}
}
