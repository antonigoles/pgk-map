#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Scene/GameObject.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Misc/Types.hpp>
#include <Engine/Support/FpsCamera.hpp>
#include <Engine/Core/Scene/SceneContext.hpp>
#include <iostream>
#include <Engine/Support/HGT/HGT.hpp>

namespace GameApp
{
	void cameraControlScript(Engine::UpdateFunctionData data) {
		static float angularSpeed = 0.001f;
		static float verticalSpeed = 50.0f;
		static bool noRepeatLock_UP = false;
		static bool noRepeatLock_DOWN = false;

		Engine::FpsCamera* camera = static_cast<Engine::FpsCamera*>(static_cast<Engine::Updateable*>(data.sourcePointer));
		auto window = data.sceneContext->glfwWindow;

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !noRepeatLock_UP) {
			angularSpeed += 0.001f;
			verticalSpeed += 15.0f;
			noRepeatLock_UP = true;
		}

		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !noRepeatLock_DOWN) {
			angularSpeed = std::max(0.001f, angularSpeed - 0.001f);
			verticalSpeed = std::max(50.0f, verticalSpeed - 15.0f);
			noRepeatLock_DOWN = true;
		}

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) noRepeatLock_UP = false;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) noRepeatLock_DOWN = false;

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

		translation.y = 0.0;

		if (glm::length2(translation) > 0.0) {
			translation = glm::normalize(translation);

			// calculate angular rotation on 
			Engine::HGT* hgt = (Engine::HGT*)camera->get_ref("hgt");
			glm::vec3 cross = glm::cross(translation, glm::vec3(0.0f, 1.0f, 0.0f)); 

			hgt->transform.setRotation(
				glm::normalize(
					glm::angleAxis(data.deltaTime * angularSpeed * (camera->transform.getPosition().y / hgt->transform.getScale().x), cross) 
					* hgt->transform.getRotation()
				)
			);
		}

		glm::vec3 dy = glm::vec3(0.0, 0.0, 0.0);

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			dy += glm::vec3{0, 1.0f, 0};
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			dy -= glm::vec3{0, 1.0f, 0};
		}

		camera->transform.setPosition(
			camera->transform.getPosition() + dy * verticalSpeed * data.deltaTime
		);
	}
}
