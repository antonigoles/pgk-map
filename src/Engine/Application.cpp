#include "Engine/Core/OpenGL.hpp"
#include "Engine/Core/Scene/Scene.hpp"
#include "Engine/Core/TextureRepository.hpp"
#include "Engine/Support/FpsCamera.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/trigonometric.hpp"
#include <Engine/Application.hpp>
#include <cstdlib>
#include <iostream>
#include <Engine/Dev/GlobalProfiler.hpp>

#include <Engine/Core/MeshRepository.hpp>
#include <Engine/Core/ShaderRepository.hpp>

namespace Engine {
    Application::Application(ApplicationSettings&& settings) {
        Engine::GlobalProfiler::setFrequency(settings.profilerSettings.logFrequency);
        this->applicationContext = {
            .settings = settings,
            .currentScene = nullptr,
            .glfwWindow = nullptr,
            .sceneRepository = new SceneRepository(),
            .meshRepository = new MeshRepository(),
            .shaderRepository = new ShaderRepository(),
            .textureRepository = new TextureRepository()
        };
    }

    void Application::glfw_framebuffer_size_callback(int width, int height)
    {
        glViewport(0, 0, width, height);
        this->applicationContext.settings.windowSettings.viewportWidth = width;
        this->applicationContext.settings.windowSettings.viewportHeight = height;
        this->applicationContext.currentScene->setViewportDimensions(width, height);
    }

    static void APIENTRY debugCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar *message,
        const void *userParam
    ) {
        std::cout << message << "\n";
    }

    void Application::makeGlfwWindow() {
        if (!glfwInit()) {
            std::cerr << "Nie udało się zainicjalizować GLFW\n";
            std::exit(-1);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

        GLFWwindow* window = glfwCreateWindow(
            this->applicationContext.settings.windowSettings.viewportWidth,
            this->applicationContext.settings.windowSettings.viewportHeight, 
            "Application",
            nullptr, 
            nullptr
        );

        if (!window) {
            std::cerr << "Nie udało się utworzyć okna GLFW\n";
            glfwTerminate();
            std::exit(-1);
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Nie udało się załadować GLAD\n";
            std::exit(-1);
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);

        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK); 
        // glFrontFace(GL_CCW);  

        glEnable(GL_STENCIL_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

        if (glDebugMessageCallback)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(debugCallback, nullptr);
            glDebugMessageControl(
                GL_DONT_CARE,
                GL_DONT_CARE,
                GL_DONT_CARE,
                0,
                nullptr,
                GL_TRUE
            );
        } else {
            std::cout << "no glDebugMessageCallback\n";
        }



        // glEnable(GL_FRAMEBUFFER_SRGB);

        this->applicationContext.glfwWindow = window;
    };

    void Application::checkFrameBufferSizeEvent() {
        int width, height;
        glfwGetFramebufferSize(this->applicationContext.glfwWindow, &width, &height);
        if (width != this->applicationContext.settings.windowSettings.viewportWidth 
        || height != this->applicationContext.settings.windowSettings.viewportHeight) {
            this->glfw_framebuffer_size_callback(width, height);
        }
    };

    void Application::run() {
        // FpsCamera* minimapCamera = new FpsCamera(90.0f);
        // minimapCamera->transform.setPosition({0, 50.0f, 0});
        // minimapCamera->transform.setRotation(glm::angleAxis(glm::radians(-25.0f), glm::vec3{1.0f, 0.0f, 0.0f}));

        float deltaTime = 0.0f;
        float frameTimeCount = 0;
        int frameCount = 0;

        if (this->applicationContext.currentScene->getFlag(SCENE_FLAGS::RENDER_DEBUG_UI)) {
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            ImGui_ImplGlfw_InitForOpenGL(this->applicationContext.glfwWindow, true);        
            ImGui_ImplOpenGL3_Init();
        }

        bool my_tool_active = true;
        float col[4] = {0, 0, 0};

        this->applicationContext.currentScene->prepareScene();

        while (!glfwWindowShouldClose(this->applicationContext.glfwWindow))
        {
            Engine::GlobalProfiler::openNewSection("Frame");
            double timeStamp = glfwGetTime();

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // glViewport(0, 0, 
            //     this->applicationContext.settings.windowSettings.viewportWidth, 
            //     this->applicationContext.settings.windowSettings.viewportHeight
            // );
            
            Engine::GlobalProfiler::openNewSection("CPU Code");
            this->applicationContext.currentScene->stepBy(deltaTime);
            this->applicationContext.currentScene->setLayerMask(LayerMask(Layer::DEFAULT));
            this->applicationContext.currentScene->render();

            // this->applicationContext.currentScene->setLayerMask(LayerMask(Layer::MINIMAP));
            // auto savedCamera = this->applicationContext.currentScene->getSceneContext()->camera;
            // this->applicationContext.currentScene->setCamera(minimapCamera);
            // this->applicationContext.currentScene->setViewportDimensions(300, 300);
            // glViewport(0, 0, 
            //     300, 
            //     300
            // );  

            // glClear(GL_DEPTH_BUFFER_BIT);
            // this->applicationContext.currentScene->render();

            // this->applicationContext.currentScene->setViewportDimensions(
            //     this->applicationContext.settings.windowSettings.viewportWidth, 
            //     this->applicationContext.settings.windowSettings.viewportHeight
            // );
            // this->applicationContext.currentScene->setCamera(savedCamera);

            Engine::GlobalProfiler::closeLastSection();
            glfwPollEvents();

            if (this->applicationContext.currentScene->getFlag(SCENE_FLAGS::RENDER_DEBUG_UI)) {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                this->applicationContext.currentScene->renderDebugUI();
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
            
            glfwSwapBuffers(this->applicationContext.glfwWindow);

            this->checkFrameBufferSizeEvent();
            deltaTime = glfwGetTime() - timeStamp;
            if (frameTimeCount > 2.0) {
                std::cout << ((float)frameCount)/(frameTimeCount) << "FPS" << "\n";
                frameCount = 0;
                frameTimeCount = 0;
            }
            // minimapCamera->transform.setRotation(
            //    glm::normalize(glm::angleAxis(deltaTime * 0.10f, glm::vec3{0, 1.0f, 0}) * minimapCamera->transform.getRotation())
            // );

            // minimapCamera->transform.setPosition(
            //    glm::vec3{0, 20.0f, 0} - 35.0f * (glm::normalize(minimapCamera->getForward() * glm::vec3(1.0f, 0.0f, 1.0f)))
            // );
            frameTimeCount += deltaTime;
            frameCount++;
            Engine::GlobalProfiler::closeLastSection();
        }
        glfwTerminate();
        if (this->applicationContext.currentScene->getFlag(SCENE_FLAGS::RENDER_DEBUG_UI)) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    };

    void Application::setScene(Scene *scene) {
        this->applicationContext.currentScene = scene;
        scene->injectApplicationContext(&this->applicationContext);
    };

    Scene* Application::createScene(std::string &&name) {
        return this->applicationContext.sceneRepository->createScene(std::move(name));
    };

    template <>
    ShaderRepository* Application::getComponent<ShaderRepository>() {
        return this->applicationContext.shaderRepository;
    };

    template <>
    MeshRepository* Application::getComponent<MeshRepository>() {
        return this->applicationContext.meshRepository;
    };

    template<>
    TextureRepository* Application::getComponent<TextureRepository>() {
        return this->applicationContext.textureRepository;
    }
}