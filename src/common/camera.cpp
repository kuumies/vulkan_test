/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::Camera struct.
 * -------------------------------------------------------------------------- */

#include "camera.h"

#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace kuu
{

glm::mat4 Camera::worldTransform() const
{
    return glm::translate(
        glm::mat4(1.0f), glm::vec3(pos)) *
        glm::mat4_cast(rotation());
}

glm::mat4 Camera::viewMatrix() const
{
    return glm::inverse(worldTransform());
}

void Camera::update()
{
    tPos  += move;
    yaw   = glm::slerp(yaw,   tYaw,   0.05f);
    pitch = glm::slerp(pitch, tPitch, 0.05f);
    roll  = glm::slerp(roll,  tRoll,  0.05f);
    pos   = glm::mix(pos, tPos, 0.1f);
}

glm::quat Camera::rotation() const
{ return yaw * pitch * roll; }

glm::mat4 Camera::projectionMatrix() const
{
    glm::mat4 m = glm::perspective(
        glm::radians(fieldOfview),
        aspectRatio,
        nearPlane,
        farPlane);
//    m[1][1] *= -1;
    glm::mat4 clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f,-1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.5f, 0.0f,
                               0.0f, 0.0f, 0.5f, 1.0f);
    return clip * m;
}

glm::mat4 Camera::cameraMatrix() const
{
    return projectionMatrix() * viewMatrix();
}

} // namespace kuu
