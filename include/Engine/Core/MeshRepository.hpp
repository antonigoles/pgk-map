#pragma once
#include <Engine/Core/Mesh.hpp>
#include <unordered_map>
#include <Engine/Core/Misc/Types.hpp>
#include <Engine/Core/Scene/GameObject.hpp>
#include <string>

namespace Engine 
{
    class MeshRepository {
    private:
        std::unordered_map<EngineID, Mesh*> meshMap;
        std::unordered_map<EngineID, unsigned int> VAOMap;
        std::unordered_map<GameObject*, EngineID> objectMap;

        unsigned int meshCounter = 0;

        EngineID getNextMeshIndex();
    public:
        std::unordered_map<EngineID, std::vector<GameObject*>> meshToGameObjects;

        MeshRepository();
        
        EngineID loadMeshFromFileWithPreprocessing(const std::string &path, glm::quat rotation);
        EngineID loadMeshFromFile(const std::string &path);

        EngineID loadMeshFromOBJFileWithPreprocessing(const std::string &path, glm::quat rotation, TextureRepository *textureRepository);
        EngineID loadMeshFromOBJFile(const std::string &path, TextureRepository *textureRepository);

        EngineID loadMesh(Mesh* mesh);
        
        unsigned int getMeshVAO(EngineID meshId); 
        
        size_t getMeshSize(EngineID meshId);

        EngineID getMeshIDByGameObject(GameObject *gameObject);

        void assignMesh(GameObject* gameObject, EngineID meshId);

        Mesh* getMeshByID(EngineID engineID);
        std::vector<Mesh*> getAllMeshes();
    };
};

