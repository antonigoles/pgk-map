#pragma once
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Mesh.hpp>
#include <Engine/Core/Math/Transform.hpp>

namespace Engine {
	// simple SymetricBox colider

	class Colider {
	private:
		glm::vec3 position;
		float heightUp;
		float heightDown;
		float radius;

		bool __isEnabled;

	public:
		Colider(
			glm::vec3 position,
			float heightUp,
			float heightDown,
			float radius
		);
		void updateByPosition(glm::vec3 position);

		void setEnabled(bool enabled);
		bool isEnabled();

		glm::vec3 getMaxP();
		glm::vec3 getMinP();
	};


	class ColiderRepository {
	private:
		std::vector<Colider*> coliders;
	
	public:
		ColiderRepository();

		Colider* registerWithMeshAndTransform(Mesh* mesh, Math::Transform transform);
		void registerColider(Colider* colider);

		bool findColision(Colider* with);
	};
}