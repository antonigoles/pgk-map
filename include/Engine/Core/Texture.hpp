#pragma once
#include <Engine/Core/OpenGL.hpp>
#include <string>

namespace Engine {
	class Texture {
	private:
		unsigned int gl_texture = 0;

		int availableUnderIndex;

		std::string path;
	public:
		Texture(unsigned int gl_texture, std::string path);

		unsigned int intgetGlTexture();

		std::string getPath();
		void setPath(const std::string& path);
		
		void setGlTexture(unsigned int gl_texture);

		int bindTextureAs(GLenum textureUnit, GLenum target);
		int bindTextureOn(int index);
		int bindTexture();
	};
};