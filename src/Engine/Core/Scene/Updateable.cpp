#include <Engine/Core/Scene/Updateable.hpp>
#include <string>

namespace Engine {
    Updateable::Updateable() {}

    void Updateable::registerUpdateFunction(ComponentUpdateFunction updateFunction) {
        this->updateFunctions.push_back(updateFunction);
    };

    void Updateable::callUpdateFunctions(float deltaTime, SceneContext* sceneContext) {
        for (auto fun : this->updateFunctions) fun({this, deltaTime, sceneContext});
    };

    void Updateable::set_vec3(const std::string &label, glm::vec3 val) {
        this->vec3map[label] = val;
    };

    glm::vec3 Updateable::get_vec3(const std::string &label) {
        // TODO: make this secure but not slow
        return this->vec3map[label];
    };

    bool Updateable::has_vec3(const std::string &label) {
        return this->vec3map.contains(label);
    };

    void Updateable::set_float(const std::string &label, float val) {
        this->floatmap[label] = val;
    };

    float Updateable::get_float(const std::string &label) {
        // TODO: make this secure but not slow
        return this->floatmap[label];
    };

    bool Updateable::has_float(const std::string &label) {
        return this->floatmap.contains(label);
    };

    void Updateable::set_ref(const std::string &label, void* val) {
        this->refmap[label] = val;
    };

    void* Updateable::get_ref(const std::string &label) {
        return this->refmap[label];
    };

    bool Updateable::has_ref(const std::string &label) {
        return this->refmap.contains(label);
    };
}