/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::Light struct.
 * -------------------------------------------------------------------------- */

#pragma once

#include <glm/vec3.hpp>

namespace kuu
{

/* -------------------------------------------------------------------------- *
   A light.
 * -------------------------------------------------------------------------- */
struct Light
{
    glm::vec3 dir = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 intensity = glm::vec3(1.0f, 1.0f, 1.0f);
};

} // namespace kuu
