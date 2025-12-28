#pragma once
#include <Engine/Core/Texture.hpp>
#include <Engine/Core/Misc/Types.hpp>
#include <string>
#include <unordered_map>

namespace Engine {
	class TextureRepository {
	private:
		std::unordered_map<EngineID, Texture*> texturesMap;

		EngineID lastId = 0;

		Texture* emptyTexture;

		EngineID getNextID();
	public:
		TextureRepository();
		std::pair<Texture*, EngineID> loadBasicFromFile(const std::string &path);
		Texture* getTexture(const EngineID& textureId);
		
		Texture* getEmptyTexture();

		void loadEmptyTexture(const std::string &path);
	};
};