#include "Engine/Core/Event/EventDispatcher.hpp"
#include "Engine/Core/Rendering/Layers.hpp"
#include "Engine/Core/Scene/SceneContext.hpp"
#include <Engine/Core/Math/Transform.hpp>
#include <Engine/Core/Scene/GameObject.hpp>
#include <Engine/Core/Math/Transform.hpp>
#include <string>

namespace Engine
{
    GameObject::GameObject() : GameObject("GameObject") {};

    GameObject::GameObject(std::string label) : EventDispatcher(), HasLayerMask(), Updateable() {
        transform = Math::Transform();
        this->label = label;
        this->v_isVisible = true;
        this->parentCluster = nullptr;
        this->v_isDisabled = false;
        this->texture = nullptr;
    }

    void GameObject::disable() {
        this->v_isDisabled = true;
    };

    void GameObject::enable() {
        this->v_isDisabled = false;
    };

    bool GameObject::isEnabled() {
        return !this->v_isDisabled;
    };

    void GameObject::hide() {
        this->v_isVisible = false;
    };

    void GameObject::show() {
        this->v_isVisible = true;
    };

    bool GameObject::isVisible() {
        return this->v_isVisible;
    };

    bool GameObject::hasParentCluster() {
        return this->parentCluster != nullptr;
    };

    void GameObject::setParentCluster(void* cluster) {
        this->parentCluster = cluster;
    };
    
    void* GameObject::getParentCluster() {
        return this->parentCluster;
    };

    void GameObject::setTexture(Texture* texture) {
        this->texture = texture;
    };

    Texture* GameObject::getTexture() {
        return this->texture;
    };
};
