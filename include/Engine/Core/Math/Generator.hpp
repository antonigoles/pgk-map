#pragma once
#include <Engine/Core/Misc/Types.hpp>
#include <Engine/Core/Mesh.hpp>

namespace Engine::Math {
    Mesh* generateCloud(int seed, int planeCount, const std::string& texture);
    Mesh* generateSphere(int precision);
    Mesh* generateMessySphere(int precision, int seed);
    Mesh* generateInverseSphere(int precision);
    float perlinNoise3D(const glm::vec3& pos, unsigned int seed);
};