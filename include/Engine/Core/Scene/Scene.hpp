#pragma once
#include <Engine/Core/Math/AABBTree.hpp>
#include <Engine/Core/Rendering/Layers.hpp>
#include <Engine/Core/SkyBox.hpp>
#include <Engine/Core/Scene/SceneContext.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Scene/GameObject.hpp>
#include <Engine/Core/Scene/Camera.hpp>
#include <Engine/Core/Scene/GameObjectCluster.hpp>
#include <Engine/Core/Scene/GameObjectRepository.hpp>
#include <Engine/Core/Scene/VFX/VolumetricParticles.hpp>
#include <Engine/Core/Scene/LightSource.hpp>
#include <Engine/Core/Rendering/Renderable.hpp>
#include <Engine/Core/Rendering/RendererContext.hpp>

namespace Engine {
    enum SCENE_FLAGS {
        RENDER_AABB_TREE = 0b1,
        RENDER_DEBUG_UI = 0b10
    };

    // Forward
    class ApplicationContext;

    class Scene : public HasLayerMask {
    private:
        int flags;

        float sunIntensity;
        glm::vec3 sunColor;
        glm::vec3 sunAmbientColor;

        AABBTree *sceneAABBTree;

        std::vector<LightSource*> lightSources;

        std::vector<GameObject*> debugObjectsPool;
        EngineID debugCube;
        EngineID debugShader;

        SceneContext sceneContext;

        std::vector<Renderable*> customRenderables;

        SkyBox *skybox;
        int v_width, v_height;

        void drawGameObjectsByShaders(const std::vector<unsigned int>& shaderIds, RendererContext* context);
        void drawGameObjectsByClusters(std::vector<GameObjectCluster*>& clusters, RendererContext* context);
        void drawParticlesBySphericalGenerators(const std::vector<SphericalVolumetricParticleGenerator*>& generators, RendererContext* context);
    public:
        Scene();
        void setViewportDimensions(int v_width, int v_height);

        void injectApplicationContext(ApplicationContext *applicationContext);

        void stepBy(float deltaTime);
        void render();
        void renderDebugUI();

        void cursorSetFocusMode();
        void cursorSetFreeMode();

        void setPolygonModeForMinorViewport();
        void setPolygonModeForMajorViewport();

        bool inputClickedOnce(int key);

        void setWindow(GLFWwindow *window);
        void setCamera(Camera *camera);
        void setSkybox(SkyBox *skybox);

        void addLightSource(LightSource* lightSource);
        void setLightSourceUniforms();

        AABBTree* getSceneAABBTree();

        void updateDebugObjects();
        void initDebugObjects();

        SceneContext* getSceneContext();
        bool getFlag(int flag);
        void setFlag(int flag);
        void unsetFlag(int flag);

        void prepareScene();

        void addRenderable(Renderable* renderable);

        template <class T>
        T* getComponent();
    };
};