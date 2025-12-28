#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Math/Transform.hpp>

namespace Engine::Math
{
    Transform::Transform() {
        position = glm::vec3(0.0f, 0.0f, 0.0f);
        rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        scale = glm::vec3(1.0f, 1.0f, 1.0f);
        this->recalculateModelMatrix();
    }

    bool Transform::isModelMatrixValid() {
        return !this->pendingModelMatrixCalculation;
    };

    void Transform::invalidateModelMatrix() {
        this->pendingModelMatrixCalculation = true;
    };

    void Transform::recalculateModelMatrix() {
        glm::mat4 model = glm::mat4(1.0f);

        // kolejność: najpierw przesunięcie, potem rotacja, potem skala
        model = glm::translate(model, this->position);
        model *= glm::mat4_cast(this->rotation);
        model = glm::scale(model, this->scale);

        this->modelMatrix = model;
    }


    glm::mat4 Transform::getModelMatrix() {
        if (this->isModelMatrixValid()) return this->modelMatrix;
        this->recalculateModelMatrix();
        return this->modelMatrix; 
    };

    void Transform::setPosition(glm::vec3 position) {
        this->invalidateModelMatrix();
        this->position = position;
    };

    void Transform::setRotation(glm::quat rotation) {
        this->invalidateModelMatrix();
        this->rotation = rotation;
    };

    void Transform::setScale(glm::vec3 scale) {
        this->invalidateModelMatrix();
        this->scale = scale;
    };

    void Transform::setScale(float scale) {
        this->invalidateModelMatrix();
        this->scale = glm::vec3(scale, scale, scale);
    }

    glm::vec3 Transform::getPosition() {
        return this->position;
    };

    glm::quat Transform::getRotation() {
        return this->rotation;
    };

    glm::vec3 Transform::getScale() {
        return this->scale;
    };

    Transform Transform::base() {
        return Transform{};
    };
};
