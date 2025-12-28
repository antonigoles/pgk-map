#include <Engine/Support/TpsCamera.hpp>
#include <Engine/Core/Math/Transform.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Math/Math.hpp>
#include <iostream>

namespace Engine
{
    static float scroll_dx = 0.0f;
    static float scroll_dy = 0.0f;

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        scroll_dx = xoffset;
        scroll_dy = yoffset;
    }


    TpsCamera::TpsCamera(float FOV) : Camera(FOV) {
        this->lastXpos = 0.0;
        this->lastYpos = 0.0; 
        this->isRotating = false;

        this->yaw = 0.0f;
        this->pitch = 0.0f;
        this->sensitivity = 0.07f;
        this->distance = 5.0f;
    };

    void TpsCamera::lookAt(glm::vec3 target) {};

    glm::mat4 TpsCamera::getViewMatrix() {
        glm::vec3 position = this->transform.getPosition();
        glm::quat rotation = this->transform.getRotation();

        // Obliczamy kierunek, w który patrzy kamera
        glm::vec3 front = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 up    = rotation * glm::vec3(0.0f, 1.0f,  0.0f);

        return glm::lookAt(position, position + front, up);
    }

    float TpsCamera::getFOV() {
        return this->FOV;
    };

    void TpsCamera::handleMouseInput(GLFWwindow * window, float deltaTime) {
        static bool init = false;

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);   

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS && !this->isRotating) {
            this->lastXpos = xpos;
            this->lastYpos = ypos;
            this->isRotating=true;
        }

        if (!(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)) {
            this->isRotating=false;
            this->lastXpos = xpos;
            this->lastYpos = ypos;
        } 

        if (!init) {
            init = true;
            // glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            this->lastXpos = xpos;
            this->lastYpos = ypos;
            this->isRotating=true;

            glfwSetScrollCallback(window, scroll_callback);
        }

        this->distance = std::min(15.0f, std::max(this->distance - scroll_dy, 1.0f));

        float dx = (this->lastXpos - xpos);
        float dy = (this->lastYpos - ypos);

        this->yaw = this->yaw + sensitivity * dx;
        this->pitch = std::min(89.0f, std::max(-89.0f, this->pitch + this->sensitivity * dy));

        glm::quat sidewaysRotation = glm::angleAxis(
            glm::radians(this->yaw),
            glm::vec3(0, 1, 0)
        );
        
        glm::quat upwardsRotation = glm::angleAxis(
            glm::radians(this->pitch),
            glm::vec3(1, 0, 0)
        );

        // rotate the forward vector and set our camera position to target - forward * distance
        // and then rotate the camera to look at our player

        glm::vec3 forward = {0.0f, 0.0f,  -1.0f};
        forward = sidewaysRotation * upwardsRotation * forward;

        // if (isRotating) {

        this->transform.setPosition(
            targetPosition - forward * distance
        );

        this->transform.setRotation(
            Math::lookAtQuat(this->transform.getPosition(), targetPosition, {0.0f, 1.0f, 0.0f})
        );
        
        lastXpos = xpos;
        lastYpos = ypos;   

        scroll_dy = 0;
        scroll_dx = 0;
        // }
    };

    void TpsCamera::syncCameraAndTarget(Math::Transform& target) {
        targetPosition = target.getPosition();
    };

    // TODO: Fix
    glm::vec3 TpsCamera::getForward() { 
        auto temp = this->transform.getRotation() * glm::vec3(0.0f, 0.0f,  -1.0f); 
        temp.y = 0;
        return glm::normalize(temp);
    };

    glm::vec3 TpsCamera::getUp() { 
        return glm::vec3(0.0f, 1.0f,  0.0f); 
    };

    glm::vec3 TpsCamera::getRight() { 
        auto temp = this->transform.getRotation() * glm::vec3(1.0f, 0.0f,  0.0f); 
        temp.y = 0;
        return glm::normalize(temp);
    };
};