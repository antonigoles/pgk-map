#include "Engine/Core/Texture.hpp"
#include <Engine/Core/TextureRepository.hpp>
#include <stb_image.h>
#include <Engine/Core/OpenGL.hpp>
#include <iostream>
#include <Engine/Core/Misc/StrLib.hpp>

namespace Engine {
	TextureRepository::TextureRepository() {

	};

	EngineID TextureRepository::getNextID() {
		return ++this->lastId;
	};

	void TextureRepository::loadEmptyTexture(const std::string &path) {
		this->emptyTexture = this->loadBasicFromFile(path).first;
	};

	Texture* TextureRepository::getEmptyTexture() {
		return this->emptyTexture;
	};

	std::pair<Texture*, EngineID> 
	TextureRepository::loadBasicFromFile(const std::string &path) {
		Texture* texture = new Texture(0, path);

		std::vector<std::string> pathParts;
		split_string(path, '.', pathParts);
		bool RGBA = pathParts.back() == "png";

		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true); 
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, RGBA ? STBI_rgb_alpha : STBI_rgb);
		if (!data) {
			std::cout << "Failed to load texture: " << path << "\n";
			std::exit(-1);
		}
		unsigned int gl_texture;
		glGenTextures(1, &gl_texture); 
		glActiveTexture(GL_TEXTURE);
		glBindTexture(GL_TEXTURE_2D, gl_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, RGBA ? GL_RGBA : GL_RGB, width, height, 0, RGBA ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);

		texture->setGlTexture(gl_texture);

		EngineID id = this->getNextID();
		this->texturesMap[id] = texture;
		return {texture, id};
	};

	Texture* TextureRepository::getTexture(const EngineID& textureId) {
		return this->texturesMap[textureId];
	};
};