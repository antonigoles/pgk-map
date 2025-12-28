#pragma once
#include <Engine/Core/Texture.hpp>
#include <vector>
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Material.hpp>

namespace Engine {
	struct MeshDrawStep {
		std::string which;
		int offset;
		int amountToDraw;
	};

	class Mesh {
	private:
		std::vector<float> data;
		Material *material;
		std::vector<MeshDrawStep> drawingSteps;
		std::vector<bool> isSmoothShaded;

		bool shouldInsertAsSmooth;
	public:
		Mesh();

		void setMaterial(Material *material);
		Material* getMaterial();

		void enableSmoothTriangleInsertion();
		void disableSmoothTriangleInsertion();

		void push_full_info_triangle(glm::vec3 vertices[3], glm::vec3 normals[3], glm::vec2 texCoords[3]);
		void push_triangle_with_normals(glm::vec3 vertices[3], glm::vec3 normals[3]);
		void push_triangle(glm::vec3 vertices[3]);

		void push_full_info_vertex(glm::vec3 vertex, glm::vec3 normal, glm::vec2 texCoords);
		void push_vertex_with_normal(glm::vec3 vertex, glm::vec3 normal);
		void push_vertex(glm::vec3 vertex);

		void addDrawingStep(MeshDrawStep step);
		const std::vector<MeshDrawStep>& getDrawingSteps();

		void rotate(glm::quat rot);

		void calculateSmoothNormals();

		void sanityCheck();

		std::pair<glm::vec3, glm::vec3> getCuboid(float padding);

		unsigned int size();
		float* get_data();

		void transform_vertices(std::function<void(float*, float*, float*)>);

		void foreach_triangle(std::function<void(const glm::vec3[3], const glm::vec3[3], const glm::vec2[3])> callback);

		std::vector<unsigned int> serialize_to_v1_format();
	};
}