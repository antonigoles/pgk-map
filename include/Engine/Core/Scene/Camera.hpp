#pragma once
#include <Engine/Core/Math/Transform.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Scene/Updateable.hpp>

namespace Engine
{
    class Camera : public Updateable {
    protected:
        float FOV;
    
    public:
        Math::Transform transform;
        
        Camera(float FOV);

        virtual glm::mat4 getViewMatrix();
        virtual float getFOV();

        virtual void handleMouseInput(GLFWwindow * window, float deltaTime);
        virtual void syncCameraAndTarget(Math::Transform& target);

        virtual void lookAt(glm::vec3 target);

        virtual glm::vec3 getForward();
        virtual glm::vec3 getUp();
        virtual glm::vec3 getRight();
    };
};
