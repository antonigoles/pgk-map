#include "Engine/Core/Colisions.hpp"
#include "Engine/Core/Material.hpp"
#include "Engine/Core/Math/AABBTree.hpp"
#include "Engine/Core/MemoryTransporter.hpp"
#include "Engine/Core/Mesh.hpp"
#include "Engine/Core/MeshRepository.hpp"
#include "Engine/Core/OpenGL.hpp"
#include "Engine/Core/Rendering/Layers.hpp"
#include "Engine/Core/Scene/GameObjectRepository.hpp"
#include "Engine/Core/ShaderRepository.hpp"
#include "Engine/Core/SkyBox.hpp"
#include "imgui.h"
#include <misc/cpp/imgui_stdlib.h>
#include <Engine/Core/Scene/Scene.hpp>
#include <Engine/Core/Scene/GameObject.hpp>
#include <Engine/Core/Scene/VFX/VolumetricParticles.hpp>
#include <Engine/Core/Math/Math.hpp>
#include <Engine/Core/Scene/GameObjectCluster.hpp>
#include <Engine/Dev/GlobalProfiler.hpp>
#include <Engine/Application.hpp>
#include <iostream>
#include <format>

namespace Engine {
    Scene::Scene() : v_width(1920), v_height(1080), HasLayerMask() {
        this->skybox = nullptr;
        this->sceneContext = {
            .window = nullptr,
            .camera = nullptr,
            .gameObjectClusterRepository = new GameObjectClusterRepository(),
            .gameObjectRepository = new GameObjectRepository(),
            .volumetricParticleGeneratorRepository = new VolumetricParticleGeneratorRepository(),
            .memoryTransporter = new MemoryTransporter(),
            .coliderRepository = new ColiderRepository()
        };
        this->sceneContext.gameObjectClusterRepository->injectSceneContext(&this->sceneContext);
        this->sceneContext.gameObjectRepository->injectSceneContext(&this->sceneContext);
        this->sceneContext.volumetricParticleGeneratorRepository->injectSceneContext(&this->sceneContext);

        this->sunIntensity = 1.24f;
        this->sunColor = {209.0/255.0, 156.0/255.0, 96.0/255.0};
        this->sunAmbientColor = {39.0/255.0, 88.0/255.0, 193.0/255.0};

        this->flags = 0b0;
        // this->sceneAABBTree = new AABBTree();
    };
    
    void Scene::setViewportDimensions(int v_width, int v_height) {
        this->v_width = v_width;
        this->v_height = v_height;
    };

    bool Scene::inputClickedOnce(int key) {
        static std::unordered_map<int, int> keyPreviousFrameState;
        auto state = glfwGetKey(this->sceneContext.glfwWindow, key);
        
        if (!keyPreviousFrameState.contains(key) || keyPreviousFrameState[key] != GLFW_PRESS) {
            if (state == GLFW_PRESS) {
                keyPreviousFrameState[key] = state;
                return true;
            }
        }

        keyPreviousFrameState[key] = state;
        return false;
    };

    void Scene::renderDebugUI() {
        static bool init = false;
        static bool my_tool_active = true;

        if (!this->getFlag(SCENE_FLAGS::RENDER_DEBUG_UI)) return;

        ImGui::Begin("Sun Controller", &my_tool_active, ImGuiWindowFlags_MenuBar);
        ImGui::ColorEdit3("Sun Ambient Color", &this->sunAmbientColor[0]);
        ImGui::ColorEdit3("Sun Color", &this->sunColor[0]);
        ImGui::SliderFloat("Sun intensity", &this->sunIntensity, 0.0f, 100.0f);
        ImGui::End();

        auto meshRepository = this->sceneContext.meshRepository;
        auto gameObjectRepository = this->sceneContext.gameObjectRepository;

        int j = 10000;

        for (auto gameObject : gameObjectRepository->getAllGameObjects()) {
            if (gameObject->hasParentCluster()) continue;
            auto meshID = meshRepository->getMeshIDByGameObject(gameObject);
            auto mesh = meshRepository->getMeshByID(meshID);
            
            auto material = mesh->getMaterial();    

            std::string c_path = material->getPath();
            const char *title = c_path.c_str();

            if (gameObject->label == "GameObject") continue;
            std::string gameObjectName = std::format("{} ({})", gameObject->label, reinterpret_cast<unsigned long>(gameObject));
            const char *gotitle = gameObjectName.c_str();

            ImGui::Begin(gotitle, &my_tool_active, ImGuiWindowFlags_MenuBar);
            ImGui::Text("[*] Transform");

            float x = gameObject->transform.getPosition().x;
            float y = gameObject->transform.getPosition().y;
            float z = gameObject->transform.getPosition().z;

            ImGui::InputFloat("Position.x", &x, -100.0f, 100.0f);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                gameObject->transform.setPosition({x, y, z});
            }
            ImGui::InputFloat("Position.y", &y, -100.0f, 100.0f);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                gameObject->transform.setPosition({x, y, z});
            }
            ImGui::InputFloat("Position.z", &z, -100.0f, 100.0f);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                gameObject->transform.setPosition({x, y, z});
            }
            if (ImGui::Button("x +")) x += 0.10;
            ImGui::SameLine();
            if (ImGui::Button("x -")) x -= 0.10;
            ImGui::SameLine();
            if (ImGui::Button("y +")) y += 0.10;
            ImGui::SameLine();
            if (ImGui::Button("y -")) y -= 0.10;
            ImGui::SameLine();
            if (ImGui::Button("z +")) z += 0.10;
            ImGui::SameLine();
            if (ImGui::Button("z -")) z -= 0.10;
            gameObject->transform.setPosition({x, y, z});

            float sx = gameObject->transform.getScale().x;
            float sy = gameObject->transform.getScale().y;
            float sz = gameObject->transform.getScale().z;

            ImGui::InputFloat("Scale.x", &x, -100.0f, 100.0f);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                gameObject->transform.setScale({sx, sy, sz});
            }
            ImGui::InputFloat("Scale.y", &y, -100.0f, 100.0f);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                gameObject->transform.setScale({sx, sy, sz});
            }
            ImGui::InputFloat("Scale.z", &z, -100.0f, 100.0f);
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                gameObject->transform.setScale({sx, sy, sz});
            }
            if (ImGui::Button("sx +")) sx += 0.10;
            ImGui::SameLine();
            if (ImGui::Button("sx -")) sx -= 0.10;
            ImGui::SameLine();
            if (ImGui::Button("sy +")) sy += 0.10;
            ImGui::SameLine();
            if (ImGui::Button("sy -")) sy -= 0.10;
            ImGui::SameLine();
            if (ImGui::Button("sz +")) sz += 0.10;
            ImGui::SameLine();
            if (ImGui::Button("sz -")) sz -= 0.10;
            gameObject->transform.setScale({sx, sy, sz});


            // ImGui::Begin(title, &my_tool_active, ImGuiWindowFlags_MenuBar);
            ImGui::Text("[*] Material Setting: %s", title);
            ImGui::PushID(j++);

            bool materialUpdate = false;

            int i = 0;
            for (auto block : material->getMTLBlocks()) {
                ImGui::PushID(i++);
                ImGui::Text("%s", block->name.c_str());
                materialUpdate |= ImGui::ColorEdit3("Ambient", &block->Ka[0]);
                materialUpdate |= ImGui::ColorEdit3("Diffusion", &block->Kd[0]);
                materialUpdate |= ImGui::ColorEdit3("Specular color", &block->Ks[0]);
                materialUpdate |= ImGui::SliderFloat("Specular exponent", &block->Ns, 0.0f, 250.0f);
                materialUpdate |= ImGui::SliderFloat("Opaqueness", &block->d, 0.0f, 1.0f);
                // ImGui::SliderFloat("????", &block->Ni, 0.0f, 1000.0f);
                materialUpdate |= ImGui::ColorEdit3("Emmission", &block->Ke[0]);
                materialUpdate |= ImGui::ColorEdit3("Fressner coef", &block->F0[0]);
                materialUpdate |= ImGui::SliderFloat("Txt scale X", &block->map_Kd.scale[0], 0.0f, 100.0f);
                materialUpdate |= ImGui::SliderFloat("Txt scale Y", &block->map_Kd.scale[1], 0.0f, 100.0f);

                static struct ComboItem {
                    const char* label;
                    int value;
                } items[6] = {
                    { "Just color/texture",   0 },
                    { "Just diffusion",   1 },
                    { "Blinn-Phong + Lamber", 2 },
                    { "Blinn-Phong + Lamber + Whitted's Reflections", 3 },
                    { "Blinn-Phong + Lamber + Whitted's Reflections - Pseudo Glass shader", 4 },
                    { "Blinn-Phong + Lamber + Whitted's Reflections + Fresnel effects", 5 },
                };

                const char* currentLabel = nullptr;
                for (auto& it : items)
                {
                    if (it.value == block->illum) {
                        currentLabel = it.label;
                        break;
                    }
                }

                if (ImGui::BeginCombo("Illumination mode", currentLabel))
                {
                    for (auto& it : items)
                    {
                        bool is_selected = (it.value == block->illum);
                        if (ImGui::Selectable(it.label, is_selected))
                            block->illum = it.value;

                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    materialUpdate = true;
                    ImGui::EndCombo();
                }
                ImGui::PopID();
            };


            if (ImGui::Button("Save"))
            {
                material->writeToDisk("." + material->getPath());
            }

            ImGui::PopID();
            ImGui::End();

            if (materialUpdate) this->prepareScene();
        }
    };

    void Scene::stepBy(float deltaTime) {
        auto gameObjectRepository = this->sceneContext.gameObjectRepository;
        auto gameObjectClusterRepository = this->sceneContext.gameObjectClusterRepository;

        auto volumetricParticleGeneratorRepository = this->sceneContext.volumetricParticleGeneratorRepository;
        
        for (GameObject* gameObject : gameObjectRepository->getAllGameObjects()) {
            if (!gameObject->isEnabled()) continue;
            gameObject->callUpdateFunctions(deltaTime, &this->sceneContext);
        }

        auto sphericalGenerators = volumetricParticleGeneratorRepository->getAllSphericalGenerators();
        for (auto& generator : sphericalGenerators) {
            if (!volumetricParticleGeneratorRepository->stepSphericalGenerator(generator->generatorID, deltaTime)) {
                volumetricParticleGeneratorRepository->deleteSphericalGenerator(generator->generatorID);
            };
        }

        this->sceneContext.camera->handleMouseInput(this->sceneContext.glfwWindow, deltaTime);
        this->sceneContext.camera->callUpdateFunctions(deltaTime, &this->sceneContext);
    };

    void Scene::drawGameObjectsByShaders(const std::vector<unsigned int>& shaderIds, RendererContext* context) {
        auto shaderRepository = this->sceneContext.shaderRepository;
        auto gameObjectRepository = this->sceneContext.gameObjectRepository;
        auto meshRepository = this->sceneContext.meshRepository;
        int objCount = 0;

        for (auto& shaderID : shaderIds) {
            shaderRepository->useShaderWithDataByID(shaderID, {}, {});
            // this->setLightSourceUniforms();
            shaderRepository->setUniformMat4("projection", context->projection);
            shaderRepository->setUniformMat4("view", context->view);
            shaderRepository->setUniformVec3("viewPos", context->cameraPosition);
            shaderRepository->setUniformFloat("time", glfwGetTime());

            for (auto& gameObjectID : gameObjectRepository->getGameObjectIDsByShaderID(shaderID)) {
                objCount++;
                GameObject* gameObject = gameObjectRepository->getGameObject(gameObjectID);
                
                if (!this->getLayerMask().matchWith(gameObject->getLayerMask())) continue;

                if (!gameObject->isVisible() || !gameObject->isEnabled()) continue;
                // if (gameObject->label == "debug") {
                //     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                // }
                EngineID meshID = meshRepository->getMeshIDByGameObject(gameObject);
                shaderRepository->setUniformMat4("model", gameObject->transform.getModelMatrix());
                shaderRepository->setUniformVec3("objPosition", gameObject->transform.getPosition());
                shaderRepository->setUniformInt("isCluster", 0);
                shaderRepository->setUniformInt("isSkybox", 0);

                if (gameObject->label == "cloud") {
                    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    glDepthMask(GL_FALSE);
                    glDisable(GL_CULL_FACE);  
                } 

                unsigned int meshVAO = meshRepository->getMeshVAO(meshID);
                glBindVertexArray(meshVAO);

                Mesh* go_mesh = meshRepository->getMeshByID(meshID);
                if (go_mesh == nullptr) {
                    std::cout << "Could not resolve mesh\n";
                    std::exit(-1);
                }
                int sanityCheckCounter = 0;

                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                for (const MeshDrawStep& drawStep : go_mesh->getDrawingSteps()) {
                    MTL_block *block = go_mesh->getMaterial()->getMTLBlock(drawStep.which);
                    if (block == nullptr) {
                        std::cout << "Could not find the MTL block: " << drawStep.which << "\n";
                        std::exit(-1);
                    }
                    std::string blockName = go_mesh->getMaterial()->getPath() + drawStep.which;
                    shaderRepository->setUniformInt(
                        "currentMTLBlock", 
                        this->sceneContext.memoryTransporter->getMaterialBufferIndex(blockName)
                    );

                    shaderRepository->setUniformInt("cubeMap", this->skybox->getCubeMap()->bind());
                    shaderRepository->setUniformInt("ourTexture", block->map_Kd.texture->bindTexture());
                    glDrawArrays(GL_TRIANGLES, drawStep.offset, drawStep.amountToDraw);
                    sanityCheckCounter += drawStep.amountToDraw;
                }
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                if (gameObject->label == "cloud") {
                    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glDepthMask(GL_TRUE);
                    glEnable(GL_CULL_FACE);
                }
            }
        }
    }

    void Scene::drawGameObjectsByClusters(std::vector<GameObjectCluster*>& clusters, RendererContext* context) {
        auto shaderRepository = this->sceneContext.shaderRepository;
        auto meshRepository = this->sceneContext.meshRepository;

        for (auto gameObjectCluster : clusters) {
            if (!this->getLayerMask().matchWith(gameObjectCluster->getLayerMask())) continue;
            
            shaderRepository->useShaderWithDataByID(gameObjectCluster->getShader(), {}, {});
            // this->setLightSourceUniforms();

            shaderRepository->setUniformMat4("projection", context->projection);
            shaderRepository->setUniformMat4("view", context->view);
            shaderRepository->setUniformVec3("viewPos", context->cameraPosition);
            shaderRepository->setUniformInt("isCluster", 1);
            shaderRepository->setUniformInt("isSkybox", 0);

            glBindVertexArray(gameObjectCluster->getMeshVAO());
            Engine::GlobalProfiler::openNewSection("Binding SSBO");
            auto [hightLODCountToDraw, lowLODCountToDraw] = gameObjectCluster->updateAndBindSSBO({
                .transparentSorting = shaderRepository->getShaderOptions(gameObjectCluster->getShader())->isTransparent,
                .viewPosition = context->cameraPosition,
                .LODdistance = 10000.0f
            });
            Engine::GlobalProfiler::closeLastSection();
            // glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
            Mesh* go_mesh = meshRepository->getMeshByID(gameObjectCluster->getMeshID());

            if (gameObjectCluster->gameObjects[0]->label == "cloud") {
                // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glDepthMask(GL_FALSE);
                glDisable(GL_CULL_FACE);  
            } 
            for (const MeshDrawStep& drawStep : go_mesh->getDrawingSteps()) {
                MTL_block *block = go_mesh->getMaterial()->getMTLBlock(drawStep.which);
                if (block == nullptr) {
                    std::cout << "Could not find the MTL block: " << drawStep.which << "\n";
                    std::exit(-1);
                }
                std::string blockName = go_mesh->getMaterial()->getPath() + drawStep.which;
                shaderRepository->setUniformInt(
                    "currentMTLBlock", 
                    this->sceneContext.memoryTransporter->getMaterialBufferIndex(blockName)
                );

                shaderRepository->setUniformInt("ourTexture", block->map_Kd.texture->bindTexture());
                shaderRepository->setUniformInt("cubeMap", this->skybox->getCubeMap()->bind());

                glDrawArraysInstanced(GL_TRIANGLES, drawStep.offset, drawStep.amountToDraw, hightLODCountToDraw);
            }

            if (gameObjectCluster->gameObjects[0]->label == "cloud") {
                // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glDepthMask(GL_TRUE);
                glEnable(GL_CULL_FACE); 
            } 
        }
        // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    }

    void Scene::drawParticlesBySphericalGenerators(
        const std::vector<SphericalVolumetricParticleGenerator*>& generators, 
        RendererContext* context
    ) {
        auto shaderRepository = this->sceneContext.shaderRepository;
        auto meshRepository = this->sceneContext.meshRepository;

        for (auto generator : generators) {
            shaderRepository->useShaderWithDataByID(generator->shaderID, {}, {});
            shaderRepository->setUniformMat4("projection", context->projection);
            shaderRepository->setUniformMat4("view", context->view);
            shaderRepository->setUniformVec3("viewPos", context->cameraPosition);
            EngineID meshVAO = meshRepository->getMeshVAO(generator->particleMeshId);
            glBindVertexArray(meshVAO);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, generator->dynamicSSBO);
            glDrawArraysInstanced(GL_TRIANGLES, 0, meshRepository->getMeshSize(generator->particleMeshId), generator->particleCount - generator->killedParticles);
        }
    }

    void Scene::render() {
        Engine::GlobalProfiler::openNewSection("Projection matrix calculation");
        glm::mat4 projection = glm::perspective(
            glm::radians(this->sceneContext.camera->getFOV() / 2.0f), 
            ((float)this->v_width) / ((float)this->v_height), 0.01f, 550.0f
        );
        Engine::GlobalProfiler::closeLastSection();

        auto shaderRepository = this->sceneContext.shaderRepository; 
        auto meshRepository = this->sceneContext.meshRepository; 
        auto gameObjectClusterRepository = this->sceneContext.gameObjectClusterRepository;
        auto gameObjectRepository = this->sceneContext.gameObjectRepository;
        auto volumetricParticleGeneratorRepository = this->sceneContext.volumetricParticleGeneratorRepository;
        
        Engine::GlobalProfiler::openNewSection("Game Objects rendering");

        RendererContext* rendererContext = new RendererContext{
            .view = this->sceneContext.camera->getViewMatrix(),
            .projection = projection,
            .cameraPosition = this->sceneContext.camera->transform.getPosition(),
            .shaderRepository = this->sceneContext.shaderRepository
        };

        std::vector<unsigned int> transparentShaders;
        std::vector<unsigned int> nonTransparentShaders;
        for (auto& shaderID : gameObjectRepository->getShadersUsedByGameObjects()) {
            if (shaderRepository->getShaderOptions(shaderID)->isTransparent) {
                transparentShaders.push_back(shaderID);
            } else {
                nonTransparentShaders.push_back(shaderID);
            }
        }

        drawGameObjectsByShaders(nonTransparentShaders, rendererContext);
        
        Engine::GlobalProfiler::closeLastSection();

        // Draw clusters
        std::vector<GameObjectCluster*> nonTransparentClusters;
        std::vector<GameObjectCluster*> transparentClusters;

        for (auto cluster : gameObjectClusterRepository->getClusters()) {
            if (shaderRepository->getShaderOptions(cluster->getShader())->isTransparent) {
                transparentClusters.push_back(cluster);
            } else {
                nonTransparentClusters.push_back(cluster);
            }
        }

        Engine::GlobalProfiler::openNewSection("Clusters rendering");
        drawGameObjectsByClusters(nonTransparentClusters, rendererContext);
        Engine::GlobalProfiler::closeLastSection();

        // render particles
        Engine::GlobalProfiler::openNewSection("Particles rendering");
        drawParticlesBySphericalGenerators(volumetricParticleGeneratorRepository->getAllSphericalGenerators(), rendererContext);
        Engine::GlobalProfiler::closeLastSection();

        // render renderables
        for (auto renderable : this->customRenderables) {
            renderable->render(rendererContext);
        }

        // draw skybox now
        if (this->skybox != nullptr && this->getLayerMask().matchWith(this->skybox->getLayerMask())) {
            glDepthFunc(GL_LEQUAL);
            shaderRepository->useShaderWithDataByID(this->skybox->getShader(), {}, {});
            glm::mat4 view = glm::mat4(glm::mat3(this->sceneContext.camera->getViewMatrix()));
            shaderRepository->setUniformMat4("projection", projection);
            shaderRepository->setUniformMat4("view", view);
            shaderRepository->setUniformInt("isSkybox", 1);
            glBindVertexArray(this->skybox->getSkyboxVAO());
            shaderRepository->setUniformInt("cubeMap", this->skybox->getCubeMap()->bind());
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthFunc(GL_LESS);
        }

        // Render transparent stuff

        Engine::GlobalProfiler::openNewSection("Transparent objects rendering");
        drawGameObjectsByShaders(transparentShaders, rendererContext);
        Engine::GlobalProfiler::closeLastSection();

        Engine::GlobalProfiler::openNewSection("Transparent clusters rendering");
        drawGameObjectsByClusters(transparentClusters, rendererContext);
        Engine::GlobalProfiler::closeLastSection();
    };

    void Scene::setCamera(Camera *camera) {
        this->sceneContext.camera = camera;
    };

    void Scene::setWindow(GLFWwindow *window) {
        this->sceneContext.glfwWindow = window;
    };

    void Scene::cursorSetFocusMode() {
        glfwSetInputMode(this->sceneContext.glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    };

    void Scene::cursorSetFreeMode() {
        glfwSetInputMode(this->sceneContext.glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    };

    void Scene::injectApplicationContext(ApplicationContext *applicationContext) {
        this->sceneContext.currentScene = this;
        this->sceneContext.glfwWindow = applicationContext->glfwWindow;
        this->sceneContext.sceneRepository = applicationContext->sceneRepository;
        this->sceneContext.meshRepository = applicationContext->meshRepository;
        this->sceneContext.shaderRepository = applicationContext->shaderRepository;
        this->sceneContext.textureRepository = applicationContext->textureRepository;
    };

    void Scene::addLightSource(LightSource* lightSource) {
        this->lightSources.push_back(lightSource);
    };

    void Scene::setSkybox(SkyBox* skybox) {
        this->skybox = skybox;
    }

    template <>
    GameObjectRepository* Scene::getComponent<GameObjectRepository>() {
        return this->sceneContext.gameObjectRepository;
    };

    template<>
    GameObjectClusterRepository* Scene::getComponent<GameObjectClusterRepository>() {
        return this->sceneContext.gameObjectClusterRepository;
    }

    template<>
    ColiderRepository* Scene::getComponent<ColiderRepository>() {
        return this->sceneContext.coliderRepository;
    }


    bool Scene::getFlag(int flag) {
        return (bool)(this->flags & flag);
    };

    void Scene::setFlag(int flag) {
        this->flags |= flag;
    };

    void Scene::unsetFlag(int flag) {
        this->flags &= !flag;
    };

    void Scene::initDebugObjects() {
        this->sceneAABBTree->buildTree();
        auto debugShader = this->sceneContext.shaderRepository->shaderProgramFromDirectory(
            "./assets/shaders/debug", { 
                .isTransparent = true,
                .hasGeometryShader = false,
            }
        );
        auto cubeMesh = this->sceneContext.meshRepository->loadMeshFromOBJFile(
            "./assets/meshes/large/cube.obj", 
            this->sceneContext.textureRepository
        );
        Engine::AABBTree* aabbTree = new Engine::AABBTree();
        auto cubeMeshObject = this->sceneContext.meshRepository->getMeshByID(cubeMesh);
        this->debugCube = cubeMesh;
        this->debugShader = debugShader;

        cubeMeshObject->transform_vertices([](float* x, float *y, float *z) {
            *x = (*x + 1.0f) / 2.0f;
            *y = (*y + 1.0f) / 2.0f;
            *z = (*z + 1.0f) / 2.0f;
        });

        auto transformedCubeMesh =this->sceneContext.meshRepository->loadMesh(cubeMeshObject);

        for (auto node : this->sceneAABBTree->getAllLeafNodes()) {
            auto [go, _] = this->sceneContext.gameObjectRepository->createGameObject(transformedCubeMesh, debugShader);
            this->debugObjectsPool.push_back(go);
            go->label = "debug";
        }
    };

    void Scene::updateDebugObjects() {
        int i = 0;
        for (auto node : this->sceneAABBTree->getAllLeafNodes()) {
            if (this->debugObjectsPool.size() <= i) {
                auto [go, _] = this->sceneContext.gameObjectRepository->createGameObject(
                    this->debugCube, this->debugShader
                );
                go->label = "debug";
                this->debugObjectsPool.push_back(go);
            }
            auto minP = node->getMinP();
            auto maxP = node->getMaxP();
            this->debugObjectsPool[i]->transform.setPosition(minP);
            this->debugObjectsPool[i]->transform.setScale(maxP-minP);
            i++;
        }
        while (i < this->debugObjectsPool.size()) {
            this->debugObjectsPool[i]->transform.setPosition({0,0,0});
            this->debugObjectsPool[i]->transform.setScale(0.0f);
            i++;
        }
    }

    AABBTree* Scene::getSceneAABBTree() {
        return this->sceneAABBTree;
    };

    void Scene::prepareScene() {
         for (auto customRenderable : this->customRenderables) {
            customRenderable->render_init(&this->sceneContext);
        }
        
        MemoryTransporter* transporter = this->sceneContext.memoryTransporter;
        transporter->cleanUp();

        transporter->registerGlobalBlock({
            .sunColor = glm::vec4(this->sunColor, 0.0f),
            .sunAmbientColor = glm::vec4(this->sunAmbientColor, 0.0f),
            .numLights = (int)this->lightSources.size(),
            .sunIntensity = this->sunIntensity
        });

        for (auto lightSource : this->lightSources) {
            transporter->registerLight({
                .position = glm::vec4(lightSource->position, 0.0f),
                .color = glm::vec4(lightSource->color, 0.0f),
                .intensity = lightSource->intensity,
                .power = lightSource->power,
            });
        }   

        // fetch materials from meshes
        for (auto& mesh : this->sceneContext.meshRepository->getAllMeshes()) {
            auto mat = mesh->getMaterial();
            std::string name = mat->getPath();
            for (auto& block : mat->getMTLBlocks()) {
                glm::vec2 txtScale = block->map_Kd.scale.size() == 1 ? 
                            glm::vec2(block->map_Kd.scale[0]) : 
                            glm::vec2(block->map_Kd.scale[0],block->map_Kd.scale[1]);

                transporter->registerMaterial(name + block->name, {
                    .Ka = glm::vec4(block->Ka, 0.0f),
                    .Kd = glm::vec4(block->Kd, 0.0f),
                    .Ks = glm::vec4(block->Ks, 0.0f),
                    .Ke = glm::vec4(block->Ke, 0.0f),
                    .F0 = glm::vec4(block->F0, 0.0f),
                    .map_Kd_scale = txtScale,
                    .Ns = block->Ns,
                    .Ni = block->Ni,
                    .d = block->d,
                    .illum = block->illum,
                    .usesTexture = block->map_Kd.texture != nullptr,
                });
            }
        }
        
        transporter->initAllSSBOS();
        transporter->bindStatic();
    };

    SceneContext* Scene::getSceneContext() {
        return &this->sceneContext;
    };

    void Scene::addRenderable(Renderable* renderable) {
        this->customRenderables.push_back(renderable);
    };
};