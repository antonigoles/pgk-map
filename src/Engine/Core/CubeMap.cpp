#include "Engine/Core/Misc/StrLib.hpp"
#include <Engine/Core/CubeMap.hpp>
#include <GL/gl.h>
#include <stb_image.h>
#include <vector>
#include <iostream>

namespace Engine {
	CubeMap::CubeMap(unsigned int gl_cubemapID) : gl_cubemapID(gl_cubemapID), availableUnderIndex(-1) {
	};

	CubeMap* CubeMap::createFromPathPattern(const std::string& pattern, CubeMapLoaderOptions options) {
		// we exepect to find "{face}" clause in the pattern string
		std::vector<std::tuple<std::string, GLenum>> faceVector = {
			{"lf", GL_TEXTURE_CUBE_MAP_POSITIVE_X},
			{"rt", GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
			{"up", GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
			{"dn", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
			{"ft", GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
			{"bk", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z}
		};

		unsigned int cubemapID;
		glGenTextures(1, &cubemapID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

		bool RGBA = options.isRGBA;
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(false);
		for (auto [face, glFace] : faceVector) {
			std::string path = replace_variable(pattern, "face", face);
			unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, RGBA ? STBI_rgb_alpha : STBI_rgb);
			if (!data) {
				std::cout << "Failed to load texture: " << path << "\n";
				std::exit(-1);
			}
			// TODO: handle RGBA here
			glTexImage2D(glFace, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}

		if (options.generateMipMaps) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glGenerateMipmap(GL_TEXTURE_2D);
		} else {
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
		} 

		CubeMap* cubeMap = new CubeMap(cubemapID);

		return cubeMap;
	};

	int CubeMap::bind() {
		static int lastBind = 0;
		if (this->availableUnderIndex == -1) {
			glActiveTexture(GL_TEXTURE0 + lastBind);
			glBindTexture(GL_TEXTURE_CUBE_MAP, this->gl_cubemapID);
			this->availableUnderIndex = lastBind;
			lastBind++;	
		}
		return this->availableUnderIndex;
	}

	int CubeMap::bindAt(int index) {
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->gl_cubemapID);
		this->availableUnderIndex = index;
		return this->availableUnderIndex;
	};
};