#include <Engine/Core/Texture.hpp>
#include <Engine/Core/OpenGL.hpp>
#include <string>
#include <iostream>

namespace Engine {
	Texture::Texture(unsigned int gl_texture, std::string path) 
	: gl_texture(gl_texture), path(path), availableUnderIndex(-1) {};

	void Texture::setGlTexture(unsigned int gl_texture) {
		this->gl_texture = gl_texture;
	}

	unsigned int Texture::intgetGlTexture() {
		return this->gl_texture;
	};

	int Texture::bindTexture() { // auto binding
		// static int lastBind = 8; // 8 textures and 8 cubemaps
		// if(this->availableUnderIndex==-1) {
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, this->gl_texture);
		this->availableUnderIndex = 8;
			// lastBind++;
		// }
		return this->availableUnderIndex;
	}

	// you shouldn't use this
	int Texture::bindTextureOn(int index) {
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, this->gl_texture);
		return index;
	};

	// you shouldn't use this
	int Texture::bindTextureAs(GLenum textureUnit, GLenum target) {
		glActiveTexture(textureUnit);
		glBindTexture(target, this->gl_texture);
		return textureUnit - GL_TEXTURE0;
	}

	std::string Texture::getPath() {
		return this->path;
	}

	void Texture::setPath(const std::string& path) {
		this->path = path;
	}
};