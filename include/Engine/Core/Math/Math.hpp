#pragma once
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Misc/Types.hpp>
#include <Engine/Core/Mesh.hpp>

namespace Engine::Math
{
   glm::quat lookAtQuat(glm::vec3 eye, glm::vec3 target, glm::vec3 up);
   void meshRotate(Mesh *mesh, glm::quat rot);
   glm::quat rotateTowards(glm::quat currentRot, glm::vec3 targetDir, float t);
   glm::vec3 angleToEarthPoint(float latitude, float altitude, float longitude);
   float getRandom(float min, float max);

   extern int seed;
}
