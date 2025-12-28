#include <Engine/Core/SceneRepository.hpp>

namespace Engine {
	SceneRepositoryEntry::SceneRepositoryEntry(std::string&& name, Scene* scene) 
		: scene(scene), name(name) {};

	SceneRepository::SceneRepository() {};

	Scene* SceneRepository::createScene(std::string&& name) {
		SceneRepositoryEntry* entry = new SceneRepositoryEntry(std::move(name), new Scene());
		this->scenes.insert(entry);
		return entry->scene;
	};

	Scene* SceneRepository::getSceneByName(const std::string& name) {
		Scene* searchResult = nullptr;
		for (auto& entry : this->scenes) {
			if (entry->name == name) {
				searchResult = entry->scene;
				break;
			}
		}
		return searchResult;
	}

	Scene* SceneRepository::loadSceneFromFile(const std::string& path) {
		// TODO: Implement
		return nullptr;
	};
}