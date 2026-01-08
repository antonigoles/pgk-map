#include <Engine/Core/Scene/GameObject.hpp>
#include <Engine/Core/Scene/GameObjectCluster.hpp>
#include <Engine/Core/Colisions.hpp>
#include <Engine/Core/Math/Math.hpp>
#include "Engine/Core/Rendering/Layers.hpp"
#include "Engine/Support/TpsCamera.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include <Engine/Core/Scene/LightSource.hpp>
#include <Engine/Core/SkyBox.hpp>
#include <Engine/Core/Scene/Scene.hpp>
#include <Engine/Support/FpsCamera.hpp>
#include <Engine/Application.hpp>
#include <Engine/Core/MeshRepository.hpp>
#include <Engine/Core/ShaderRepository.hpp>
#include <Engine/Core/Scene/GameObjectRepository.hpp>
#include <Engine/Core/OS/Serializer.hpp>
#include <Engine/Core/Mesh.hpp>
#include <Engine/Core/Math/Generator.hpp>
#include <GameApp/Scripts/Scripts.hpp>
#include <Engine/Core/TextureRepository.hpp>
#include <Engine/Core/Material.hpp>
#include <Engine/Core/Event/Event.hpp>
#include <Engine/Support/HGT/HGT.hpp>
#include <iostream>

namespace GameApp
{
    void setupDebugScene(Engine::Application *application, Engine ::Scene* scene) {
        application->setScene(scene);

        Engine::Camera* camera = new Engine::TpsCamera(90.0f);

        scene->setCamera(camera);
        camera->transform.setPosition({0.0f, 500.0f, 0.0f});
        // camera->transform.setRotation(glm::angleAxis(3.14f / 4.0f, glm::vec3{1, 0, 0}));

        auto meshRepository = application->getComponent<Engine::MeshRepository>();
        auto shaderRepository = application->getComponent<Engine::ShaderRepository>();
        auto gameObjectRepository = scene->getComponent<Engine::GameObjectRepository>();
        auto gameObjectClusterRepository = scene->getComponent<Engine::GameObjectClusterRepository>();
        auto textureRepository = application->getComponent<Engine::TextureRepository>();

        auto cloudShader = shaderRepository->shaderProgramFromDirectory("./assets/shaders/universal-cloud", { 
            .isTransparent = true,
            .hasGeometryShader = false,
        });

        auto girlMesh = meshRepository->loadMeshFromOBJFile("./assets/meshes/large/cloud0.obj", textureRepository);
        auto [cloud1, _2] = gameObjectRepository->createGameObject(girlMesh, cloudShader);
        cloud1->transform.setPosition({-1.0f, 0.0f, 0.0f});
        cloud1->transform.setScale({1.0f, 1.0f, 1.0f});
        cloud1->label = "cloud";

        // camera->syncCameraAndTarget(cloud1->transform);

        scene->setFlag(Engine::SCENE_FLAGS::RENDER_DEBUG_UI);

        // skybox
        auto skyboxDefaultShader = shaderRepository->shaderProgramFromDirectory("./assets/shaders/skybox-default", { 
            .isTransparent = false,
            .hasGeometryShader = false
        });
        auto skybox = Engine::SkyBox::createFromPathPattern("./assets/skyboxes/pack/20250717_210621_0774_{face}.png", skyboxDefaultShader);
        scene->setSkybox(skybox);
    }

    void setupGameScene(Engine::Application *application, Engine ::Scene* scene, float startLongtitude, float startLatitude) {
        application->setScene(scene);

        Engine::FpsCamera* camera = new Engine::FpsCamera(90.0f);
        camera->registerUpdateFunction(GameApp::cameraControlScript);

        scene->setCamera(camera);
        camera->transform.setPosition({0, 100050.0f, 0});

        auto meshRepository = application->getComponent<Engine::MeshRepository>();
        auto shaderRepository = application->getComponent<Engine::ShaderRepository>();
        auto gameObjectRepository = scene->getComponent<Engine::GameObjectRepository>();
        auto gameObjectClusterRepository = scene->getComponent<Engine::GameObjectClusterRepository>();
        auto textureRepository = application->getComponent<Engine::TextureRepository>();

        auto screenShader = shaderRepository->shaderProgramFromDirectory("./assets/shaders/screen", { 
            .isTransparent = false,
            .hasGeometryShader = false,
        });

        application->setScreenShader(screenShader);

        textureRepository->loadEmptyTexture("./assets/textures/notexture.png");

        // load HGT
        Engine::HGT* hgt = Engine::HGT::fromDataSource("/mnt/d6e94da6-c1d5-4615-9695-ec204b9e3102/pgk-map/hgt_gigant");
        // Engine::HGT* hgt = Engine::HGT::fromDataSource("./assets/M33");
        scene->addRenderable(hgt);
        camera->set_ref("hgt", hgt);

        auto up = glm::vec3(0.0f, 1.0f, 0.0f);

        auto sleza = glm::vec2(50.86501694, 16.70881694);
        auto dolnyslask = glm::vec2(51.0, 17.0f);

        auto custom = glm::vec2(startLongtitude, startLatitude);

        auto dir = glm::normalize(Engine::Math::angleToEarthPoint(custom.x, 0.0, custom.y));

        // std::cout << "================== roty\n";
        std::cout << glm::orientedAngle(up, dir, up) << "\n";
        // std::cout << dir.x << " " << dir.y << " " << dir.z << "\n";

        hgt->transform.setRotation(
            glm::normalize(
                glm::angleAxis(
                    -glm::orientedAngle(up, dir, up), 
                    glm::normalize(glm::cross(up, dir))
                )
            )
        );

        hgt->transform.setScale(100000.0f);
        hgt->transform.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));

        auto cloudShader = shaderRepository->shaderProgramFromDirectory("./assets/shaders/universal", { 
            .isTransparent = false,
            .hasGeometryShader = false,
        });

        auto girlMesh = meshRepository->loadMeshFromOBJFile("./assets/meshes/large/sphere.obj", textureRepository);
        auto [cloud1, _2] = gameObjectRepository->createGameObject(girlMesh, cloudShader);
        cloud1->transform.setPosition({0.0f, 0.0f, 0.0f});
        cloud1->transform.setScale(1.0f);

        // camera->syncCameraAndTarget(hgt->transform);
        scene->setFlag(Engine::SCENE_FLAGS::RENDER_DEBUG_UI);

        // skybox
        auto skyboxDefaultShader = shaderRepository->shaderProgramFromDirectory("./assets/shaders/skybox-default", { 
            .isTransparent = false,
            .hasGeometryShader = false
        });
        auto skybox = Engine::SkyBox::createFromPathPattern("./assets/skyboxes/black_{face}.png", skyboxDefaultShader);
        // auto skybox = Engine::SkyBox::createFromPathPattern("./assets/skyboxes/pack/20250717_210621_0774_{face}.png", skyboxDefaultShader);
        scene->setSkybox(skybox);
    }

    void init(float startLongtitude, float startLatitude) {
        Engine::Application application = Engine::Application(
            Engine::ApplicationSettings{
                .profilerSettings = {
                    .logFrequency = 10000,
                },
                .windowSettings = {
                    .viewportWidth = 1920,
                    .viewportHeight = 1080
                }
            }
        );
        application.makeGlfwWindow();

        // Engine::Scene* debugScene = application.createScene("DebugScene");
        // setupDebugScene(&application, debugScene);

        Engine::Scene* debugScene = application.createScene("GameScene");
        setupGameScene(&application, debugScene, startLongtitude, startLatitude);

        application.run();
    };
};
