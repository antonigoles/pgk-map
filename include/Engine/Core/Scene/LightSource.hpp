#pragma once
#include <Engine/Core/OpenGL.hpp>

namespace Engine {
	class LightSource {
	public:
		glm::vec3 position;
	    glm::vec3 color;
	    float intensity;
	    float power;
	};
};