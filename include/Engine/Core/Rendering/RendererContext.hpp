#pragma once
#include <Engine/Core/ShaderRepository.hpp>

namespace Engine {
	class RendererContext {
	    public: 
	        glm::mat4 view;
	        glm::mat4 projection;
	        glm::vec3 cameraPosition;

			ShaderRepository* shaderRepository;
	};
}