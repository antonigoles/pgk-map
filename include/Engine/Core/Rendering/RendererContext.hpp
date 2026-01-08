#pragma once
#include <Engine/Core/ShaderRepository.hpp>
#include <Engine/Core/Scene/Camera.hpp>

namespace Engine {
	class RendererContext {
	    public: 
	        glm::mat4 view;
	        glm::mat4 projection;
	        glm::vec3 cameraPosition;

			ShaderRepository* shaderRepository;
			Camera* camera;
	};
}