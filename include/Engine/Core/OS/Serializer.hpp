#pragma once
#include <string>
#include <Engine/Core/Mesh.hpp>

namespace Engine {
	class Serializer {
	public:
		Serializer() = delete;

		static void writeMeshAsBinaryStreamOfFloats(const std::string& path, Mesh *mesh);
		static Mesh* readMeshFromBinaryFile(const std::string& path);
	};
};