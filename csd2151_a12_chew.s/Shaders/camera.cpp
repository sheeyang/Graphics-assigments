/*!*****************************************************************************
\file CG/camera.cpp
\author Vadim Surov (vsurov@digipen.edu)
\date 12/23/2025 (MM/DD/YYYY)
\brief Camera implementation for the framework: computes view/projection matrices
       and processes orbiting/walking input (cursor and scroll).
*******************************************************************************/
#include "camera.h"

#include <glm/gtc/matrix_transform.hpp> // Utility functions for matrix transformations

namespace cg
{

    // Sets the camera type and updates the target if it's orbiting
    void Camera::setType(CameraType cameraType)
    {
        this->cameraType = cameraType;
        if (cameraType == CameraType::ORBITING)
            target = { 0.0f, 0.0f, 0.0f }; // Orbiting cameras always target the origin
    }

    // Updates the shader program with camera position
    void Camera::setUniforms(Program* pProgram) const
    {
        pProgram->setUniform("camera.position", position); // Pass camera position to the shader
    }

    // Returns the view matrix for the camera using the "look at" method
    glm::mat4 Camera::getLookAt(glm::vec3 center /*= { 0.0f, 0.0f, 0.0f }*/, glm::vec3 up /*= { 0.0f, 1.0f, 0.0f }*/) const
    {
        return glm::lookAt(position, target, up);
    }

    // Returns the perspective projection matrix based on the field of view and aspect ratio
    glm::mat4 Camera::getPerspective(float aspect /*= 1*/) const
    {
        return glm::perspective(glm::radians(fieldOfView), aspect, near, far);
    }

    // Handles cursor movement to adjust camera orientation
    void Camera::onCursor(double xoffset, double yoffset)
    {
        const float PI05 = glm::pi<float>() / 2.0f; // Half of pi for clamping

        if (xoffset == 0.0 && yoffset == 0.0)
			return; // No movement, no need to update

        if (cameraType == CameraType::ORBITING)
        {
            // Calculate spherical coordinates for orbiting movement
            const float r = glm::sqrt(position.x * position.x +
                position.y * position.y + position.z * position.z); // Calculate radial distance

            if (r < std::numeric_limits<float>::epsilon())
                return; // Position at origin, cannot compute spherical coordinates

            float alpha = glm::asin(position.y / r); // Vertical angle
            float betta = std::atan2f(position.x, position.z); // Horizontal angle

            // Adjust angles based on cursor offset
            if (yoffset < 0.0)
                alpha += -0.02f;
            else if (yoffset > 0.0)
                alpha += 0.02f;

            if (xoffset < 0.0)
                betta += 0.05f;
            else if (xoffset > 0.0)
                betta += -0.05f;

            // Clamp vertical angle
            alpha = glm::clamp(alpha, -PI05 + 0.01f, PI05 - 0.01f);

            // Update position based on spherical coordinates
            position.x = r * glm::cos(alpha) * glm::sin(betta);
            position.y = r * glm::sin(alpha);
            position.z = r * glm::cos(alpha) * glm::cos(betta);
        }
        else if (cameraType == CameraType::WALKING)
        {
            // Calculate spherical coordinates for walking movement
            const float r = glm::sqrt(
                (target.x - position.x) * (target.x - position.x) +
                (target.y - position.y) * (target.y - position.y) + 
                (target.z - position.z) * (target.z - position.z));
            float alpha = glm::asin((target.y - position.y) / r);
            float betta = std::atan2f((target.x - position.x), (target.z - position.z));

            // Adjust angles based on cursor offset
            if (yoffset < 0.0)
                alpha += -0.02f;
            else if (yoffset > 0.0)
                alpha += 0.02f;

            if (xoffset < 0.0)
                betta += -0.05f;
            else if (xoffset > 0.0)
                betta += 0.05f;

            // Clamp vertical angle
            alpha = glm::clamp(alpha, -PI05 + 0.01f, PI05 - 0.01f);

            // Update target based on spherical coordinates
            target.x = position.x + r * glm::cos(alpha) * glm::sin(betta);
            target.y = position.y + r * glm::sin(alpha);
            target.z = position.z + r * glm::cos(alpha) * glm::cos(betta);
        }
    }

    // Handles scroll input to adjust zoom or camera position
    void Camera::onScroll(double xoffset, double yoffset)
    {
        if (cameraType == CameraType::ORBITING)
        {
            // Calculate the distance from the origin and adjust it based on scroll input
            float r = glm::sqrt(position.x * position.x + position.y * position.y + position.z * position.z);
            const float alpha = glm::asin(position.y / r);
            const float betta = std::atan2f(position.x, position.z);

            r += yoffset > 0.0f ? -1.0f : 1.0f; // Zoom in or out
            if (r < 1.0f) r = 1.0f; // Clamp minimum distance

            // Update position based on new distance
            position.x = r * glm::cos(alpha) * glm::sin(betta);
            position.y = r * glm::sin(alpha);
            position.z = r * glm::cos(alpha) * glm::cos(betta);
        }
        else if (cameraType == CameraType::WALKING)
        {
            // Walk forward/backward in full world space along the current view direction.
            const glm::vec3 forward = glm::normalize(target - position);
            const glm::vec3 velocity = yoffset > 0.0f ? forward : -forward;
            position += velocity;
            target += velocity;
        }
    }
}
