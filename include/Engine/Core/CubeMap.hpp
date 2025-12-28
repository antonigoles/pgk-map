#pragma once
#include <Engine/Core/Texture.hpp>
#include <string>

namespace Engine {
	class CubeMapLoaderOptions {
	public:
		bool isRGBA;
		bool generateMipMaps;
	};

	class CubeMap {
	private:
		unsigned int gl_cubemapID;

		int availableUnderIndex;

		CubeMap(unsigned int gl_cubemapID);
	public:
		CubeMap() = delete;

		int bind();
		int bindAt(int index);

		static CubeMap* createFromPathPattern(const std::string& root, CubeMapLoaderOptions options);
	};
};