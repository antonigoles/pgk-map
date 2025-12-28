#pragma once
#include <vector>
#include <Engine/Core/Misc/Types.hpp>

namespace Engine
{   
    class Updateable {
    private:
        std::vector<ComponentUpdateFunction> updateFunctions;
    
    public:
        Updateable();

        void registerUpdateFunction(ComponentUpdateFunction updateFunction);
        void callUpdateFunctions(float deltaTime, SceneContext* sceneContext);
    };
}
