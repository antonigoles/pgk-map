#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine::Math
{
    class Transform {
    private:
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
        glm::mat4 modelMatrix;

        bool pendingModelMatrixCalculation = false;

        void recalculateModelMatrix();
        void invalidateModelMatrix();
        bool isModelMatrixValid();
    public:
        Transform();

        glm::vec3 getPosition();
        glm::quat getRotation();
        glm::vec3 getScale();

        void setPosition(glm::vec3 position);
        void setRotation(glm::quat rotation);
        void setScale(glm::vec3 scale);
        void setScale(float scale);

        glm::mat4 getModelMatrix();

        static Transform base();
    };
};
