#pragma once
#include <Engine/Core/OpenGL.hpp>

namespace Engine {

	// Data that will depend on index but is static

	// keep in mind to align this
	struct GLSL_MTL_block {
	    glm::vec4 Ka;
	    glm::vec4 Kd;
	    glm::vec4 Ks;
	    glm::vec4 Ke;
	    glm::vec4 F0;
	    glm::vec2 map_Kd_scale;
	    float Ns;
	    float Ni;
	    float d;
	    int illum;
	    int usesTexture;

	    float _pad5;
	};

	// Universal data - static
	struct GLSL_GLOBAL_BLOCK {
		glm::vec4 sunColor;
		glm::vec4 sunAmbientColor;
		int numLights;
		float sunIntensity;

		float _pad2[2];
	};

	struct GLSL_LIGHT_BLOCK {
		glm::vec4 position;
	   	glm::vec4 color;
	    float intensity;
	    float power;

	    float _pad2[2];
	};

	class MemoryTransporter {
	private:
		unsigned int materialBufferSSBO;
		std::vector<std::pair<std::string, GLSL_MTL_block>> materialBuffer;

		unsigned int lightBufferSSBO;
		std::vector<GLSL_LIGHT_BLOCK> lightBuffer;

		unsigned int globalBlockBufferSSBO;
		GLSL_GLOBAL_BLOCK globalBlock;

	public:
		MemoryTransporter();

		void registerLight(GLSL_LIGHT_BLOCK block);
		void registerGlobalBlock(GLSL_GLOBAL_BLOCK block);

		unsigned int registerMaterial(const std::string& name, GLSL_MTL_block block);
		unsigned int getMaterialBufferIndex(const std::string& name);

		void cleanUp();
		void initAllSSBOS();
		void bindStatic();
	};
};