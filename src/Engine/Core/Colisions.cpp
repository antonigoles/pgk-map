#include <Engine/Core/Colisions.hpp>

namespace Engine {
	Colider::Colider(
		glm::vec3 position,
		float heightUp,
		float heightDown,
		float radius
	) : position(position), heightUp(heightUp), heightDown(heightDown), radius(radius) {
		__isEnabled = true;
	};


	void Colider::setEnabled(bool enabled) {
		this->__isEnabled = enabled;
	};

	bool Colider::isEnabled() {
		return this->__isEnabled;
	};

	void Colider::updateByPosition(glm::vec3 position) {
		this->position = position;
	};


	glm::vec3 Colider::getMaxP() {
		return this->position + glm::vec3{
			radius, heightUp, radius
		};
	};

	glm::vec3 Colider::getMinP() {
		return this->position + glm::vec3{
			-radius, heightDown, -radius
		};
	};

	ColiderRepository::ColiderRepository() {};

	void ColiderRepository::registerColider(Colider* colider) {
		this->coliders.push_back(colider);
	};

	Colider* ColiderRepository::registerWithMeshAndTransform(Mesh* mesh, Math::Transform transform) {
		auto cuboid = mesh->getCuboid(0.0f);

		auto maxP = transform.getScale() * cuboid.second;
		auto minP = transform.getScale() * cuboid.first;

		Colider* colider = new Colider(
			transform.getPosition(),
			std::max(std::max(std::abs(minP.z), std::abs(minP.x)), std::max(maxP.z, maxP.x)),
			maxP.y,
			minP.y
		);
		this->coliders.push_back(colider);
		return colider;
	};

	bool ColiderRepository::findColision(Colider* with) {
		std::vector<Colider*> result;
		for (auto other : this->coliders) {
			if (!other->isEnabled()) continue;
			if (other == with) continue;
			if (glm::all(glm::lessThanEqual(with->getMinP(), other->getMaxP())) && 
           		glm::all(glm::greaterThanEqual(with->getMaxP(), other->getMinP()))) return true;
		}
		return false;
	};
};