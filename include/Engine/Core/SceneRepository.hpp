#pragma once
#include <Engine/Core/Scene/Scene.hpp>
#include <string>

namespace Engine {
	class SceneRepositoryEntry {
	public:
		SceneRepositoryEntry(std::string&& name, Scene* scene);
		std::string name;
		Scene* scene;
	};

	class SceneRepository {
	private:
		std::unordered_set<SceneRepositoryEntry*> scenes;

	public:
		SceneRepository();

		Scene* createScene(std::string&& name);
		Scene* getSceneByName(const std::string& name);

		Scene* loadSceneFromFile(const std::string& path);
	};
};