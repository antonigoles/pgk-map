#pragma once
#include <Engine/Core/Misc/Types.hpp>
#include <Engine/Core/Scene/GameObject.hpp>
#include <unordered_map>
#include <Engine/Core/Scene/SceneContext.hpp>

namespace Engine 
{
    class GameObjectRepository : public SceneContextConsumer {
    private:
        std::vector<GameObject*> orderedGameObjects;
        std::unordered_map<EngineID, GameObject*> gameObjectMap;
        std::unordered_map<unsigned int, std::vector<EngineID>> shaderToObjectIDsMap;

        EngineID gameObjectCounter = 0;

        EngineID getNextGameObjectIndex();
    public:
        GameObjectRepository();

        GameObject* getGameObject(EngineID engineID);

        void linkShaderWithGameObject(unsigned int shaderID, EngineID gameObjectID);
        std::vector<EngineID> getGameObjectIDsByShaderID(unsigned int shaderID);
        std::vector<unsigned int> getShadersUsedByGameObjects();


        std::vector<GameObject*> getAllGameObjects();
        std::pair<GameObject*, EngineID> createGameObject(EngineID mesh, EngineID shaderProgram);
        std::pair<GameObject*, EngineID> createEmptyGameObject();
    };
};