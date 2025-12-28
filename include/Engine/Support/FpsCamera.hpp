#pragma once
#include <Engine/Core/Math/Transform.hpp>
#include <Engine/Core/Scene/Camera.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Scene/Updateable.hpp>

namespace Engine
{
    class FpsCamera : public Camera {
    public:        
        FpsCamera(float FOV);

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
