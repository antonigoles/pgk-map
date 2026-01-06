#pragma once
#include <vector>
#include <Engine/Core/Misc/Types.hpp>
#include <string>

namespace Engine
{   
    class Updateable {
    private:
        std::vector<ComponentUpdateFunction> updateFunctions;
        
        std::unordered_map<std::string, glm::vec3> vec3map;
        std::unordered_map<std::string, float> floatmap;
        std::unordered_map<std::string, void*> refmap;
    public:
        Updateable();

        void registerUpdateFunction(ComponentUpdateFunction updateFunction);
        void callUpdateFunctions(float deltaTime, SceneContext* sceneContext);

        void set_vec3(const std::string &label, glm::vec3 val);
        glm::vec3 get_vec3(const std::string &label);
        bool has_vec3(const std::string &label);

        void set_float(const std::string &label, float val);
        float get_float(const std::string &label);
        bool has_float(const std::string &label);

        void set_ref(const std::string &label, void* val);
        void* get_ref(const std::string &label);
        bool has_ref(const std::string &label);
    };
}
