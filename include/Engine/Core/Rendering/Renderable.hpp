#pragma once
#include <Engine/Core/Rendering/RendererContext.hpp>
#include <Engine/Core/Scene/SceneContext.hpp>
#include <Engine/Core/Math/Transform.hpp>

namespace Engine
{
    class Renderable {
    public:
        Math::Transform transform;
        
        Renderable();

        virtual void render_init(SceneContext *sceneContext);
        virtual void render(RendererContext *context);
    };
}
