#pragma once
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Texture.hpp>
#include <string>
#include <Engine/Core/TextureRepository.hpp>

namespace Engine {

	class TextureMap {
	public:
		Texture *texture;
		std::vector<float> scale; // -s 
		float bumpMultiplier; // -bm
	};

	class MTL_block {
	public:
		static MTL_block* make_default(TextureRepository *textureRepository);

		std::string name;
		float Ns; // specular exponent
		glm::vec3 Ka;
		glm::vec3 Kd;
		glm::vec3 Ks;
		glm::vec3 Ke;
		glm::vec3 F0;
		float Ni;
		float d;
		int illum;
		TextureMap map_Kd;
		TextureMap map_Bump;
	};

	class Material {
	private:
		std::vector<MTL_block*> blocks;

		std::string path;

		Material();
	public:
		void addBlock(MTL_block* block);

		void mergeWith(Material* material);

		MTL_block* getMTLBlock(const std::string& name);

		std::vector<MTL_block*> getMTLBlocks();

		void setPath(const std::string& path);
		void writeToDisk(const std::string& path);
		std::string getPath();

		static Material* loadMTLFile(const std::string &mtlFile, TextureRepository *textureRepository);

		static Material* createBase(TextureRepository *textureRepository);
	};
};