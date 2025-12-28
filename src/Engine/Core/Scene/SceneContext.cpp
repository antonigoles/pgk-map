#include <Engine/Core/Scene/SceneContext.hpp>

namespace Engine {
    SceneContextConsumer::SceneContextConsumer() {
    	this->sceneContext = nullptr;
    };

    void SceneContextConsumer::injectSceneContext(SceneContext* sceneContext) {
    	this->sceneContext = sceneContext;
    };
}