#pragma once
#include <string>
#include <glm/vec2.hpp>
#include <unordered_map>
#include <glad/glad.h>
#include <Engine/Core/Misc/Types.hpp>

namespace Engine {
    class ShaderOptions {
    public:
        bool isTransparent;
        bool hasGeometryShader;
    };

    class ShaderRepository {
    private:
        unsigned int lastLoadedShader = -1;

        std::unordered_map<std::string, int> shaderMap;
        std::unordered_map<GLint, ShaderOptions> shaderOptions;

        std::unordered_map<GLint, std::unordered_map<std::string, GLint>> locationCache;

        unsigned int compileShader(const std::string& source, GLenum type);
        std::string loadFile(const std::string &path);

        GLint getUnfiromLocation(const std::string &name);
    public:
        ShaderRepository();

        unsigned int getShaderProgram(std::string label);

        unsigned int shaderProgramFromDirectory(const std::string& directory, ShaderOptions options);

        ShaderOptions* getShaderOptions(GLint id);

        unsigned int loadShaderProgramAs(
            std::string label,
            std::string vertexCodePath, 
            std::string fragmentCodePath,
            std::string geometryCodePath,
            ShaderOptions options
        );

        void useShaderWithDataByLabel(
            const std::string& label,
            const std::unordered_map<std::string, float>& floatVariables,
            const std::unordered_map<std::string, glm::vec2>& vec2Variables
        );

        void useShaderWithDataByID(
            unsigned int shaderID,
            const std::unordered_map<std::string, float>& floatVariables,
            const std::unordered_map<std::string, glm::vec2>& vec2Variables
        );

        void setUniformMat4(const std::string &name, const glm::mat4 &mat);
        void setUniformVec3(const std::string &name, const glm::vec3 &vec);
        void setUniformVec2(const std::string &name, const glm::vec2 &vec);
        void setUniformFloat(const std::string &name, const float &value);
        void setUniformInt(const std::string &name, const int &value);
    };
};