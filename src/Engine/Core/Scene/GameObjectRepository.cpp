#include <Engine/Core/Scene/GameObjectRepository.hpp>
#include <Engine/Core/Misc/Types.hpp>
#include <Engine/Core/Scene/GameObject.hpp>
#include <unordered_map>
#include <Engine/Core/Scene/SceneContext.hpp>
#include <Engine/Core/MeshRepository.hpp>
#include <Engine/Core/ShaderRepository.hpp>

namespace Engine 
{
    GameObjectRepository::GameObjectRepository() {};

    EngineID GameObjectRepository::getNextGameObjectIndex() {
        return this->gameObjectCounter++;
    };

    void GameObjectRepository::linkShaderWithGameObject(unsigned int shaderID, EngineID gameObjectID) {
        this->shaderToObjectIDsMap[shaderID].push_back(gameObjectID);
    };
    
    std::vector<EngineID> GameObjectRepository::getGameObjectIDsByShaderID(unsigned int shaderID) {
        return this->shaderToObjectIDsMap[shaderID];
    };

    std::vector<unsigned int> GameObjectRepository::getShadersUsedByGameObjects() {
        std::vector<unsigned int> result;
        for (auto& [shaderID, vec] : this->shaderToObjectIDsMap) {
            result.push_back(shaderID);
        }
        return result;
    };


    std::pair<GameObject*, EngineID> GameObjectRepository::createEmptyGameObject() {
        EngineID id = this->getNextGameObjectIndex();
        GameObject* gameObject = new GameObject();
        this->gameObjectMap[id] = gameObject;
        this->orderedGameObjects.push_back(gameObject);
        return {gameObject, id};
    }

    std::vector<GameObject*> GameObjectRepository::getAllGameObjects() {
        return this->orderedGameObjects; 
    };
    
    std::pair<GameObject*, EngineID> GameObjectRepository::createGameObject(
        EngineID mesh, 
        EngineID shaderProgram
    ) {
        EngineID id = this->getNextGameObjectIndex();
        GameObject* gameObject = new GameObject();
        
        this->gameObjectMap[id] = gameObject;
        this->orderedGameObjects.push_back(gameObject);

        this->linkShaderWithGameObject(shaderProgram, id);
        
        this->sceneContext->meshRepository->assignMesh(gameObject, mesh);

        return {gameObject, id};
    };

    GameObject* GameObjectRepository::getGameObject(EngineID engineID) {
        return this->gameObjectMap[engineID];
    };
};