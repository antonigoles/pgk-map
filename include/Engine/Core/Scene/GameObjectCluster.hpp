#pragma once
#include <Engine/Core/Rendering/Layers.hpp>
#include <Engine/Core/Scene/GameObject.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Scene/SceneContext.hpp>

namespace Engine 
{
    struct SSBO_Transform {
        glm::mat4x4 model;
    };

    struct ssboUpdateOptions {
        bool transparentSorting;
        glm::vec3 viewPosition;
        float LODdistance;
    };

    enum ClusterUpdateStrategy {
        STATIC_NO_DYNAMIC_LOD = 0,
        DYNAMIC = 1,
    };

    class GameObjectCluster : public SceneContextConsumer, public HasLayerMask {
    private:
        ClusterUpdateStrategy updateStrategy;

        GLuint meshVAO;
        GLuint SSBO;
        EngineID meshID;
        
        EngineID shaderId;

        bool ssboInitialized;

        int meshSize;
    public:
        GameObjectCluster();
        
        std::vector<GameObject*> gameObjects;
        void setMesh(EngineID meshID);
        void setShader(EngineID shaderID);

        std::pair<unsigned int, unsigned int> updateAndBindSSBO(ssboUpdateOptions options);
        void initiateSSBO();

        EngineID getShader();
        
        GLuint getMeshVAO();
        EngineID getMeshID();
        unsigned int getMeshSize();

        void addObject(GameObject *gameObject);
        void deleteObject(GameObject *gameObject);
        void setUpdateStrategy(ClusterUpdateStrategy updateStrategy);
    };


    class GameObjectClusterRepository : public SceneContextConsumer {
    private:
        std::vector<GameObjectCluster*> clusterVector;
        std::unordered_map<EngineID, GameObjectCluster*> clusterMap;

        EngineID lastId;

        EngineID getNextID();
    public:
        GameObjectClusterRepository();

        GameObjectCluster* getCluster(const EngineID& clusterID);
        std::vector<GameObjectCluster*> getClusters();
        std::pair<GameObjectCluster*, EngineID> createCluster(ClusterUpdateStrategy updateStrategy);
    };
}