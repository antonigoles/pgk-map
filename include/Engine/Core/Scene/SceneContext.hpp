#pragma once
#include "Engine/Core/MemoryTransporter.hpp"
#include "Engine/Core/TextureRepository.hpp"
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/ShaderRepository.hpp>
#include <Engine/Core/MeshRepository.hpp>
#include <Engine/Core/Colisions.hpp>

namespace Engine {
    // Forward declarations
    class Camera;
    class GameObject;
    class GameObjectClusterRepository;
    class GameObjectRepository;
    class VolumetricParticleGeneratorRepository;
    class Scene;
    class SceneRepository;

    class SceneContext {
    public:
        GLFWwindow *window;
        Camera *camera;

        GameObjectClusterRepository *gameObjectClusterRepository;
        GameObjectRepository *gameObjectRepository; 
        VolumetricParticleGeneratorRepository *volumetricParticleGeneratorRepository;
        MemoryTransporter *memoryTransporter;
        ColiderRepository *coliderRepository;

        // Application context
        Scene *currentScene;
        GLFWwindow *glfwWindow;
        SceneRepository *sceneRepository;
        MeshRepository *meshRepository;
        ShaderRepository *shaderRepository;
        TextureRepository *textureRepository;
    };

    class SceneContextConsumer {
    public:
        SceneContextConsumer();
        void injectSceneContext(SceneContext* sceneContext);
        SceneContext* sceneContext;
    };
};