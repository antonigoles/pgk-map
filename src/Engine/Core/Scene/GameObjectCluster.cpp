#include "Engine/Core/OpenGL.hpp"
#include "glm/fwd.hpp"
#include <Engine/Core/Scene/GameObjectCluster.hpp>
#include <Engine/Core/MeshRepository.hpp>
#include <Engine/Core/Scene/SceneContext.hpp>
#include <Engine/Dev/GlobalProfiler.hpp>

namespace Engine 
{
    // ************** GameObjectCluster **************

    // GLuint meshVAO;
    // GLuint SSBO;
    // int meshSize;
    // std::vector<GameObject*> gameObjects;
    
    GameObjectCluster::GameObjectCluster() : HasLayerMask() {}

    void GameObjectCluster::setMesh(EngineID meshID) {
        auto meshRepository = this->sceneContext->meshRepository;
        this->meshID = meshID;
        this->meshVAO = meshRepository->getMeshVAO(meshID);
        this->meshSize = meshRepository->getMeshSize(meshID);
    };

    EngineID GameObjectCluster::getMeshID() {
        return this->meshID;
    };

    std::pair<unsigned int, unsigned int> GameObjectCluster::updateAndBindSSBO(ssboUpdateOptions options) {
        int low = 0, high = this->gameObjects.size();
        if (this->SSBO == -1 || !(this->updateStrategy == ClusterUpdateStrategy::STATIC_NO_DYNAMIC_LOD)) {
            std::vector<std::pair<float, int>> low_lod_buffer_to_sort;
            std::vector<std::pair<float, int>> high_lod_buffer_to_sort;
            std::vector<SSBO_Transform> updatedData;

            std::vector<SSBO_Transform> lowLOD;
            std::vector<SSBO_Transform> highLOD;
            int i = 0;
            for (auto obj : this->gameObjects) {
                if (!obj->isVisible() || !obj->isEnabled()) {
                    i++;
                    continue;
                };
                auto dist = glm::length2(obj->transform.getPosition() - options.viewPosition);
                
                if (dist > options.LODdistance*options.LODdistance) { 
                    low_lod_buffer_to_sort.push_back({
                        dist, i
                    });
                } else {
                    high_lod_buffer_to_sort.push_back({
                        dist, i
                    });
                }
                i++;
            }
            
            if (options.transparentSorting) {
                auto cmprtr = [](const std::pair<float, glm::mat4x4>& a, const std::pair<float, glm::mat4x4>& b) {
                    // Descending because we want to render furthest elements first
                    return a.first > b.first;
                };
                std::sort(low_lod_buffer_to_sort.begin(), low_lod_buffer_to_sort.end(), cmprtr);
                std::sort(high_lod_buffer_to_sort.begin(), high_lod_buffer_to_sort.end(), cmprtr);
            }

            
            for (auto &p : high_lod_buffer_to_sort) {
                updatedData.push_back({
                    this->gameObjects[p.second]->transform.getModelMatrix()
                });
            }

            for (auto &p : low_lod_buffer_to_sort) {
                updatedData.push_back({
                    this->gameObjects[p.second]->transform.getModelMatrix()
                });
            }

            if (this->SSBO == -1) {
                glGenBuffers(1, &this->SSBO);
            }

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->SSBO);
            glBufferData(
                GL_SHADER_STORAGE_BUFFER,
                sizeof(SSBO_Transform) * updatedData.size(), 
                updatedData.data(),
                GL_DYNAMIC_DRAW
            );

            high = high_lod_buffer_to_sort.size();
            low = low_lod_buffer_to_sort.size();
        }

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->SSBO);
        return {high, low};
    };

    void GameObjectCluster::initiateSSBO() {
        this->SSBO = -1;
    };

    void GameObjectCluster::setShader(EngineID shaderID) {
        this->shaderId = shaderID;
    };

    EngineID GameObjectCluster::getShader() {
        return this->shaderId;
    };
    
    GLuint GameObjectCluster::getMeshVAO() {
        return this->meshVAO;
    };

    unsigned int GameObjectCluster::getMeshSize() {
        return this->meshSize;
    };

    void GameObjectCluster::addObject(GameObject *gameObject) {
        this->gameObjects.push_back(gameObject);
        gameObject->setParentCluster(this);
    };

    void GameObjectCluster::deleteObject(GameObject *gameObject) {
        // TODO: Implement 
    };

    void GameObjectCluster::setUpdateStrategy(ClusterUpdateStrategy updateStrategy) {
        this->updateStrategy = updateStrategy;
    };


    // ************** GameObjectClusterRepository **************

    GameObjectClusterRepository::GameObjectClusterRepository() {
        this->lastId = 0;
    };

    EngineID GameObjectClusterRepository::getNextID() {
        return ++this->lastId;
    };

    GameObjectCluster* GameObjectClusterRepository::getCluster(const EngineID& clusterID) {
        return this->clusterMap[clusterID];
    };

    std::vector<GameObjectCluster*> GameObjectClusterRepository::getClusters() {
        return this->clusterVector;
    };

    std::pair<GameObjectCluster*, EngineID> GameObjectClusterRepository::createCluster(ClusterUpdateStrategy updateStrategy) {
        EngineID newId = this->getNextID();
        GameObjectCluster* cluster = new GameObjectCluster();
        cluster->injectSceneContext(this->sceneContext);
        cluster->setUpdateStrategy(updateStrategy);
        this->clusterMap[newId] = cluster;
        this->clusterVector.push_back(cluster);
        return {cluster, newId};
    };
}