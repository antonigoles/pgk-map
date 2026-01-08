#include "Engine/Core/Scene/Scene.hpp"
#include "Engine/Core/TextureRepository.hpp"
#include "Engine/Support/FpsCamera.hpp"
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
        this->createFBO(width, height);
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

    void renderQuad()
    {
        static unsigned int quadVAO = 0;
        static unsigned int quadVBO;
        if (quadVAO == 0)
        {
            float quadVertices[] = {
                // pozycje (x, y)   // uv (u, v)
                -1.0f,  1.0f,       0.0f, 1.0f,
                -1.0f, -1.0f,       0.0f, 0.0f,
                1.0f, -1.0f,       1.0f, 0.0f,

                -1.0f,  1.0f,       0.0f, 1.0f,
                1.0f, -1.0f,       1.0f, 0.0f,
                1.0f,  1.0f,       1.0f, 1.0f
            };

            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);

            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        }

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
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

        if (!gladLoadGL(glfwGetProcAddress)) {
            std::cerr << "Nie udało się załadować GLAD\n";
            std::exit(-1);
        }

        glEnable(GL_MULTISAMPLE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK); 
        glFrontFace(GL_CCW);  

        glEnable(GL_STENCIL_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

        // Robimy reversed Z-buffer
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GREATER);

        this->applicationContext.glfwWindow = window;
        this->createFBO(
            this->applicationContext.settings.windowSettings.viewportWidth,
            this->applicationContext.settings.windowSettings.viewportHeight
        );
    };

    void Application::createFBO(int width, int height) {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &colorTexture);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Błąd FBO!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            // Sprawdź kod błędu, np. wypisz go
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

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


            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glEnable(GL_DEPTH_TEST);

            glClearDepth(0.0f);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            
            Engine::GlobalProfiler::openNewSection("CPU Code");
            this->applicationContext.currentScene->stepBy(deltaTime);
            this->applicationContext.currentScene->setLayerMask(LayerMask(Layer::DEFAULT));
            this->applicationContext.currentScene->render();

            Engine::GlobalProfiler::closeLastSection();

            if (this->applicationContext.currentScene->getFlag(SCENE_FLAGS::RENDER_DEBUG_UI)) {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                this->applicationContext.currentScene->renderDebugUI();
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            glDisable(GL_DEPTH_TEST); 
            // glDisable(GL_SCISSOR_TEST);
            // glDisable(GL_BLEND);
            
            this->applicationContext.shaderRepository->useShaderWithDataByID(screenShader, {}, {});
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, colorTexture);
            this->applicationContext.shaderRepository->setUniformInt("screenTexture", 0);
            renderQuad();

            glfwSwapBuffers(this->applicationContext.glfwWindow);
            glfwPollEvents();

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

    void Application::setScreenShader(unsigned int screenShader) {
        this->screenShader = screenShader;
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