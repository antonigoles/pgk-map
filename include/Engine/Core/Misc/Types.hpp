#pragma once
#include <glm/glm.hpp>
#include <functional>

namespace Engine
{
    typedef unsigned int EngineID;
    class SceneContext;

    class UpdateFunctionData {
    public:
        void* sourcePointer;
        float deltaTime;
        SceneContext *sceneContext;
    };

    typedef std::function<void(UpdateFunctionData)> ComponentUpdateFunction;
};

