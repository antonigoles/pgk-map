#include "Engine/Core/TextureRepository.hpp"
#include <Engine/Core/Material.hpp>
#include <Engine/Core/Misc/StrLib.hpp>
#include <string>
#include <fstream>
#include <iostream>

namespace Engine {
	Material::Material() {};

	MTL_block* MTL_block::make_default(TextureRepository *textureRepository) {
		MTL_block* mtl_block = new MTL_block{
			.name = "default",
			.Ns = 1.0,
			.Ka = {0.0, 0.0, 0.0},
			.Kd = {0.0, 0.0, 0.0},
			.Ks = {0.0, 0.0, 0.0},
			.Ke = {0.0, 0.0, 0.0},
			.F0 = {0.03, 0.03, 0.03},
			.Ni = 1.0,
			.d = 1.0,
			.illum = 2,
			.map_Kd = {
				.texture = textureRepository->getEmptyTexture(),
				.scale = {1.0},
				.bumpMultiplier = 1.0
			},
			.map_Bump = {
				.texture = nullptr,
				.scale = {1.0},
				.bumpMultiplier = 1.0
			}
		};

		return mtl_block;
	}

	void Material::addBlock(MTL_block* block) {
		std::cout << "Adding " << block->name << " block\n";
		this->blocks.push_back(block);
	};

	void Material::mergeWith(Material* material) {
		this->path = material->path;
		for (auto& b : material->blocks) {
			this->blocks.push_back(b);
		}
	};

	MTL_block* Material::getMTLBlock(const std::string& name) {
		MTL_block* b = nullptr;
		for (MTL_block* block : this->blocks) {
			if (block->name == name) {
				b = block;
				break;
			}
		}
		return b;
	};

	std::vector<MTL_block*> Material::getMTLBlocks() {
		return this->blocks;
	};

	Material* Material::createBase(TextureRepository *textureRepository) {
		Material* mat = new Material();
		
		MTL_block* block = MTL_block::make_default(textureRepository);

		mat->addBlock(block);
		return mat;
	};

	void Material::setPath(const std::string& path) {
		std::cout << "setting path " << path << "\n";
		this->path = path;
	};

	std::string Material::getPath() {
		return this->path;
	};
	
	void Material::writeToDisk(const std::string& path) {
		std::ofstream mtlFile(path, std::ios::trunc);
		std::cout << "Saving MTL to: " << path << "\n";

		for (auto block : this->blocks) {
			mtlFile << "newmtl " << block->name << "\n";
			mtlFile << "Ns " << block->Ns << "\n";
			mtlFile << "F0 " << block->F0.x << " " << block->F0.y << " " << block->F0.z << "\n";
			mtlFile << "Ka " << block->Ka.x << " " << block->Ka.y << " " << block->Ka.z << "\n";
			mtlFile << "Kd " << block->Kd.x << " " << block->Kd.y << " " << block->Kd.z <<"\n";
			mtlFile << "Ks " << block->Ks.x << " " << block->Ks.y << " " << block->Ks.z << "\n";
			mtlFile << "Ke " << block->Ke.x << " " << block->Ke.y << " " << block->Ke.z << "\n";
			mtlFile << "Ni " << block->Ni << "\n";
			mtlFile << "d " << block->d << "\n";
			mtlFile << "illum " << block->illum << "\n";
			
			if (block->map_Kd.texture != nullptr) {
				mtlFile << "map_Kd -s";
				for (auto i : block->map_Kd.scale) {
					mtlFile << " " << i;
				}
				mtlFile << " " << block->map_Kd.texture->getPath() << "\n";
			}
			
			if (block->map_Bump.texture != nullptr) {
				mtlFile << "map_Bump -s " << block->map_Bump.bumpMultiplier;
				mtlFile << " " << block->map_Bump.texture->getPath() << "\n";
				mtlFile << "\n";
			}
		}

		mtlFile.close();
	};

	Material* Material::loadMTLFile(const std::string &mtlFile, TextureRepository *textureRepository) {
		Material* material = new Material();
		std::ifstream file(mtlFile);
        if (!file.is_open())
            throw std::runtime_error(std::string("Could not open mtl file: ") + mtlFile);

        std::string line;

        MTL_block *block = nullptr;
        material->setPath(mtlFile);

        std::vector<std::string> mtlFilePathParts;
        split_string(mtlFile, '/', mtlFilePathParts);
        mtlFilePathParts.pop_back();
        std::string rootPath = join_string(mtlFilePathParts, "/");

        while (getline (file, line)) {
        	// std::cout << "Parsing: " << line << "\n";
        	std::vector<std::string> tokens;
        	line.erase(std::remove(line.begin(), line.end(), '\n'), line.cend());
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());
            split_string(line, ' ', tokens);
            if (tokens.size() == 0) continue;

            if (tokens[0] == "newmtl") {
            	block = MTL_block::make_default(textureRepository);
            	std::vector<std::string> name_split;
            	for (int i = 1; i<tokens.size(); i++) name_split.push_back(tokens[i]);
            	block->name = join_string(name_split, " ");
                material->addBlock(block);
            	continue;
            }

            if (tokens[0] == "Ns") {
            	block->Ns = parseFloat(tokens, 1);
            	continue;
            }

            if (tokens[0] == "Ni") {
            	block->Ni = parseFloat(tokens, 1);
            	continue;
            }

            if (tokens[0] == "d") {
            	block->d = parseFloat(tokens, 1);
            	continue;
            }

            if (tokens[0] == "Ka") {
            	block->Ka = parseVec3(tokens, 1);
            	continue;
            }

            if (tokens[0] == "Kd") {
            	block->Kd = parseVec3(tokens, 1);
            	continue;
            }

            if (tokens[0] == "Ks") {
            	block->Ks = parseVec3(tokens, 1);
            	continue;
            }

            if (tokens[0] == "Ke") {
            	block->Ke = parseVec3(tokens, 1);
            	continue;
            }

            if (tokens[0] == "F0") {
            	block->F0 = parseVec3(tokens, 1);
            	continue;
            }

            if (tokens[0] == "illum") {
            	block->illum = parseInt(tokens, 1);
            	continue;
            }

            if (tokens[0] == "map_Kd") {
            	// TODO: Better path resolver
            	std::cout << "Loading map_Kd texture from: " << rootPath + "/" + tokens.back() << "\n";
            	auto [texture, _] = textureRepository->loadBasicFromFile(rootPath + "/" + tokens.back());
            	block->map_Kd = TextureMap{
            		.texture = texture,
            		.scale = parseVectorFlag(tokens, "-s", {1.0}),
            		.bumpMultiplier = 1.0,
            	};
            	texture->setPath(tokens.back());
            	continue;
            }

            if (tokens[0] == "map_Bump") {
            	// TODO: Better path resolver
            	auto [texture, _] = textureRepository->loadBasicFromFile(rootPath + "/" + tokens.back());
            	block->map_Kd = TextureMap{
            		.texture = texture,
            		.scale = {1.0},
            		.bumpMultiplier = parseVectorFlag(tokens, "-bm", {1.0})[0],
            	};
            	continue;
            }

            if (tokens[0][0] != '#') {
            	std::cout << "Unsuported material keyword detected: " << tokens[0] << "\n";
            }
        }

        file.close();
        return material;
	};
};