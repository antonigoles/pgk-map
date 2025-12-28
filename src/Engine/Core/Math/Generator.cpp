#include <Engine/Core/Misc/Types.hpp>
#include <vector>
#include <Engine/Core/OpenGL.hpp>
#include <numeric>
#include <algorithm>
#include <random>
#include <cmath>
#include <Engine/Core/Mesh.hpp>

namespace Engine::Math {
    float perlinNoise3D(const glm::vec3& pos, unsigned int seed = 0) {
        // Utwórz permutację za każdym razem (seed faktycznie działa)
        std::vector<int> p(256);
        std::iota(p.begin(), p.end(), 0);
        std::shuffle(p.begin(), p.end(), std::default_random_engine(seed));
        p.insert(p.end(), p.begin(), p.end());

        auto fade = [](float t) {
            return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
        };
        auto lerp = [](float t, float a, float b) {
            return a + t * (b - a);
        };
        auto grad = [](int hash, float x, float y, float z) {
            int h = hash & 15;
            float u = h < 8 ? x : y;
            float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
            return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
        };

        float x = pos.x;
        float y = pos.y;
        float z = pos.z;

        int X = (int)floor(x) & 255;
        int Y = (int)floor(y) & 255;
        int Z = (int)floor(z) & 255;

        x -= floor(x);
        y -= floor(y);
        z -= floor(z);

        float u = fade(x);
        float v = fade(y);
        float w = fade(z);

        int A  = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B  = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        float res = lerp(w,
            lerp(v,
                lerp(u, grad(p[AA], x, y, z),
                        grad(p[BA], x - 1, y, z)),
                lerp(u, grad(p[AB], x, y - 1, z),
                        grad(p[BB], x - 1, y - 1, z))),
            lerp(v,
                lerp(u, grad(p[AA + 1], x, y, z - 1),
                        grad(p[BA + 1], x - 1, y, z - 1)),
                lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                        grad(p[BB + 1], x - 1, y - 1, z - 1)))
        );

        return (res + 1.0f) * 0.5f; // [0,1]
    }

    Mesh* generateSphere(int precision) {
        Mesh *mesh = new Mesh();
        const float PI = 3.14159265359f;

        for (int i = 0; i < precision; i++) {
            float phi1 = PI * i / precision;
            float phi2 = PI * (i + 1) / precision;

            for (int j = 0; j < precision; j++) {
                float theta1 = 2.0f * PI * j / precision;
                float theta2 = 2.0f * PI * (j + 1) / precision;

                glm::vec3 p1(
                    sin(phi1) * cos(theta1),
                    cos(phi1),
                    sin(phi1) * sin(theta1)
                );
                glm::vec3 p2(
                    sin(phi2) * cos(theta1),
                    cos(phi2),
                    sin(phi2) * sin(theta1)
                );
                glm::vec3 p3(
                    sin(phi2) * cos(theta2),
                    cos(phi2),
                    sin(phi2) * sin(theta2)
                );
                glm::vec3 p4(
                    sin(phi1) * cos(theta2),
                    cos(phi1),
                    sin(phi1) * sin(theta2)
                );

                // trójkąt 1 (CCW)
                glm::vec3 triangle0[3] = {p1, p2, p3};
                mesh->push_triangle(triangle0);
                // trójkąt 2 (CCW)
                glm::vec3 triangle1[3] = {p1, p3, p4};
                mesh->push_triangle(triangle1);
            }
        }

        return mesh;
    }

    Mesh* generateMessySphere(int precision, int seed) {
        Mesh* sphere = Engine::Math::generateSphere(precision);
        // TODO: Implement mesh transformer
        return sphere;
    };

    Mesh* generateInverseSphere(int precision) {
        Mesh* sphere = Engine::Math::generateSphere(precision);
        // TODO: Implement mesh transformer
        return sphere;
    };

    Mesh* generateCloud(int seed, int planeCount, const std::string& texture) {
        //TODO: Implement properly
        
        Mesh* cloudMesh = new Mesh();

        float plane[] = {
            -1.0f, -1.0f, 0.0f,    0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
            1.0f, -1.0f, 0.0f,    0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,    0.0f, 0.0f, 1.0f,   0.0f, 1.0f,

            1.0f, -1.0f, 0.0f,    0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,    0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f,    0.0f, 0.0f, 1.0f,   0.0f, 1.0f
        };

        for (int i = 0; i<planeCount; i++) {

        }

        cloudMesh->getMaterial()->getMTLBlocks()[0]->name = "cloud";
        cloudMesh->addDrawingStep({
            .which = "cloud",
            .offset = 0,
            .amountToDraw = 0
        });

        return cloudMesh;
    };
};