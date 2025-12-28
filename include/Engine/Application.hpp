#pragma once
#include "Engine/Core/TextureRepository.hpp"
#include <Engine/Core/Scene/Scene.hpp>
#include <Engine/Core/SceneRepository.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/ShaderRepository.hpp>
#include <Engine/Core/MeshRepository.hpp>

namespace Engine {
    class ProfilerSettings {
    public:
        int logFrequency;
    };

    class WindowSettings {
    public:
        int viewportWidth;
        int viewportHeight;
    };

    class ApplicationSettings {
    public:
        ProfilerSettings profilerSettings;
        WindowSettings windowSettings;
    };

    class ApplicationContext {
    public:
        ApplicationSettings settings;
        Scene* currentScene;
        GLFWwindow *glfwWindow;
        SceneRepository *sceneRepository;
        MeshRepository *meshRepository;
        ShaderRepository *shaderRepository;
        TextureRepository *textureRepository;
    };

    class Application {
    private:
        ApplicationContext applicationContext;

        void checkFrameBufferSizeEvent();

        void glfw_framebuffer_size_callback(int width, int height);
    public:
        Application() = default;
        Application(ApplicationSettings&& settings);

        void makeGlfwWindow();
        void setScene(Scene *scene);
        void run();

        Scene* createScene(std::string &&name);
        
        template <class T>
        T* getComponent();
    };
};