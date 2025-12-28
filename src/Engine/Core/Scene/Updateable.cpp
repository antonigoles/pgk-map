#include <Engine/Core/Scene/Updateable.hpp>

namespace Engine {
    Updateable::Updateable() {}

    void Updateable::registerUpdateFunction(ComponentUpdateFunction updateFunction) {
        this->updateFunctions.push_back(updateFunction);
    };

    void Updateable::callUpdateFunctions(float deltaTime, SceneContext* sceneContext) {
        for (auto fun : this->updateFunctions) fun({this, deltaTime, sceneContext});
    };
}