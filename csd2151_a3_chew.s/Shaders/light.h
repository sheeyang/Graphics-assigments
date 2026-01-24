/*!*****************************************************************************
\file CG/light.cpp
\author Vadim Surov (vsurov@digipen.edu)
\date 12/23/2025 (MM/DD/YYYY)
\brief Light implementation for the framework: uploads shader uniforms, processes
       cursor/scroll to reposition the light, and updates the shadow frustum.
*******************************************************************************/
#pragma once

#include "program.h"

#include <initializer_list>
#include <vector>      // For managing multiple light components

#include <glm/glm.hpp> // For mathematical types like vec2, vec3, mat4
#include <glm/ext.hpp> // For mathematical constants like pi

namespace cg
{
    //
    // The Light struct represents a point light source in 3D space with ambient, 
    // diffuse, and specular intensities.
    //
    struct Light
    {
        glm::vec3 position;       // The position of the light in 3D space
        std::vector<glm::vec3> L; // Intensity values for ambient, diffuse, and specular components

        Light(  glm::vec3 position = { 0.0f, 5.0f, 0.0f }, // Default position is above the origin
                std::initializer_list<glm::vec3> L =       // Default light intensities
                {
                    { 0.4f, 0.4f, 0.4f },  // Ambient light intensity (La)
                    { 1.0f, 1.0f, 1.0f },  // Diffuse light intensity (Ld)
                    { 1.0f, 1.0f, 1.0f }   // Specular light intensity (Ls)
                }
            ) : position{ position }, L{ L }
        { 
        }

        // Sets the light's uniform variables in the given shader program.
        void setUniforms(Program* pProgram, int i = 0) const;

        // Handles cursor movement events to adjust the light's position.
        void onCursor(double xoffset, double yoffset, int i = 0);

        // Handles scroll events to zoom the light in/out by adjusting its radial distance.
        void onScroll(double xoffset, double yoffset, int i = 0);
    };

    // For shadow mapping
    class LightFrustum
    {
    private:
        glm::vec3 center, at, up;
        float fovy, ar, nearDist, farDist;
        double oldx, oldy;
        glm::mat4 shadowMatrix;

    public:
        LightFrustum(
            const glm::vec3& center = { 0.0f, 10.0f, 10.0f },
            const glm::vec3& at = { 0.0f, 0.0f, 0.0f },
            const glm::vec3& up = { 0.0f, 1.0f, 0.0f },
            float fovy = 45.0f,
            float ar = 1.0f,
            float nearDist = 0.5f,
            float farDist = 100.0f
        );

        glm::vec3 getPosition() const { return center; }

        const glm::mat4& getShadowMatrix() const { return shadowMatrix; }

        void onCursor(double xpos, double ypos);

        void onScroll(double xoffset, double yoffset);

        void updateShadowMatrix();

    };
}
