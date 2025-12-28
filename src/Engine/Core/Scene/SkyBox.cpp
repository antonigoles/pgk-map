#include "Engine/Core/CubeMap.hpp"
#include "Engine/Core/OpenGL.hpp"
#include <Engine/Core/SkyBox.hpp>

namespace Engine {
	SkyBox::SkyBox() : HasLayerMask() {};

	unsigned int SkyBox::getSkyboxVAO() {
		return this->skyboxVAO;
	};
	
	CubeMap* SkyBox::getCubeMap() {
		return this->cubeMap;
	};

	EngineID SkyBox::getShader() {
		return this->shaderID;
	};

	SkyBox* SkyBox::createFromPathPattern(const std::string& root, EngineID shaderID) {
		SkyBox *skybox = new SkyBox();
		skybox->shaderID = shaderID;
		float skyboxVertices[] = {
		    // positions          
		    -1.0f,  1.0f, -1.0f,
		    -1.0f, -1.0f, -1.0f,
		     1.0f, -1.0f, -1.0f,
		     1.0f, -1.0f, -1.0f,
		     1.0f,  1.0f, -1.0f,
		    -1.0f,  1.0f, -1.0f,

		    -1.0f, -1.0f,  1.0f,
		    -1.0f, -1.0f, -1.0f,
		    -1.0f,  1.0f, -1.0f,
		    -1.0f,  1.0f, -1.0f,
		    -1.0f,  1.0f,  1.0f,
		    -1.0f, -1.0f,  1.0f,

		     1.0f, -1.0f, -1.0f,
		     1.0f, -1.0f,  1.0f,
		     1.0f,  1.0f,  1.0f,
		     1.0f,  1.0f,  1.0f,
		     1.0f,  1.0f, -1.0f,
		     1.0f, -1.0f, -1.0f,

		    -1.0f, -1.0f,  1.0f,
		    -1.0f,  1.0f,  1.0f,
		     1.0f,  1.0f,  1.0f,
		     1.0f,  1.0f,  1.0f,
		     1.0f, -1.0f,  1.0f,
		    -1.0f, -1.0f,  1.0f,

		    -1.0f,  1.0f, -1.0f,
		     1.0f,  1.0f, -1.0f,
		     1.0f,  1.0f,  1.0f,
		     1.0f,  1.0f,  1.0f,
		    -1.0f,  1.0f,  1.0f,
		    -1.0f,  1.0f, -1.0f,

		    -1.0f, -1.0f, -1.0f,
		    -1.0f, -1.0f,  1.0f,
		     1.0f, -1.0f, -1.0f,
		     1.0f, -1.0f, -1.0f,
		    -1.0f, -1.0f,  1.0f,
		     1.0f, -1.0f,  1.0f
		};
		
		unsigned int cubeVBO;
		glGenVertexArrays(1, &skybox->skyboxVAO);
		glGenBuffers(1, &cubeVBO);

		glBindVertexArray(skybox->skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skybox->skyboxVAO);

		glBufferData(GL_ARRAY_BUFFER, 432, skyboxVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        CubeMap *cubeMap = CubeMap::createFromPathPattern(root, { .isRGBA = false, .generateMipMaps = false });
        skybox->cubeMap = cubeMap;

        return skybox;
	};
};