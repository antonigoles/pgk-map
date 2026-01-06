#pragma once
#include "Engine/Core/Rendering/Layers.hpp"
#include <Engine/Core/Math/Transform.hpp>
#include <Engine/Core/Misc/Types.hpp>
#include <string>
#include <unordered_map>
#include <Engine/Core/Texture.hpp>
#include <Engine/Core/Event/EventDispatcher.hpp>
#include <Engine/Core/Scene/Updateable.hpp>

namespace Engine
{
    class GameObject : public EventDispatcher, public HasLayerMask, public Updateable {
    private:
        void* parentCluster;
        bool v_isVisible;
        bool v_isDisabled;

        // this might not age well
        Texture* texture;
    public:
        Math::Transform transform;
        std::string label;
        GameObject();
        GameObject(std::string label);

        bool hasParentCluster();
        void setParentCluster(void* cluster);
        void* getParentCluster();

        void setTexture(Texture* texture);
        Texture* getTexture();
        
        void hide();
        void show();
        bool isVisible();

        void disable();
        void enable();
        bool isEnabled();
    };
};
