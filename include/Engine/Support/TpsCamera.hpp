#pragma once
#include <Engine/Core/Math/Transform.hpp>
#include <Engine/Core/Scene/Camera.hpp>
#include <Engine/Core/OpenGL.hpp>

namespace Engine
{
    class TpsCamera : public Camera {
    private:
        glm::vec3 targetPosition;

        float lastXpos;
        float lastYpos; 
        bool isRotating;

        float yaw;
        float pitch;
        float sensitivity;
        float distance;

    public:        
        TpsCamera(float FOV);

        glm::mat4 getViewMatrix() override;
        float getFOV() override;

        void handleMouseInput(GLFWwindow * window, float deltaTime) override;
        void syncCameraAndTarget(Math::Transform& target) override;

        void lookAt(glm::vec3 target) override;

        glm::vec3 getForward() override;
        glm::vec3 getUp() override;
        glm::vec3 getRight() override;
    };
};
