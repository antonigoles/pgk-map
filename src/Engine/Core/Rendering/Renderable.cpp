#include <Engine/Core/Rendering/Renderable.hpp>
#include <Engine/Core/Scene/Scene.hpp>
#include <Engine/Core/Rendering/RendererContext.hpp>
#include <iostream>

namespace Engine
{
    Renderable::Renderable() {
        this->transform = Math::Transform{};
    };
    
    void Renderable::render_init(SceneContext *sceneContext) {
        std::cerr << "Error: calling virtual method\n";
    };

    void Renderable::render(RendererContext *context) {
        std::cerr << "Error: calling virtual method\n";
    };
}
