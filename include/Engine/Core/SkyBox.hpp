#pragma once
#include "Engine/Core/Rendering/Layers.hpp"
#include <Engine/Core/CubeMap.hpp>
#include <Engine/Core/Misc/Types.hpp>

namespace Engine {
	class SkyBox : public HasLayerMask {
	private:
		CubeMap* cubeMap;
		unsigned int skyboxVAO;

		EngineID shaderID;

		SkyBox();
	
	public:
		unsigned int getSkyboxVAO();
		CubeMap* getCubeMap();
		EngineID getShader();

		static SkyBox* createFromPathPattern(const std::string& root, EngineID shaderID);
	};
};