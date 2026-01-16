/*!*****************************************************************************
\file CG/light.cpp
\author Vadim Surov (vsurov@digipen.edu)
\date 12/23/2025 (MM/DD/YYYY)
\brief Light implementation for the framework: uploads shader uniforms, processes
       cursor/scroll to reposition the light, and updates the shadow frustum.
*******************************************************************************/
#include "light.h"

#include <sstream>     // For dynamic string generation

namespace cg
{
    // Sets the light's uniform variables in the given shader program.
    void Light::setUniforms(Program* pProgram, int i /*= 0*/) const
    {
        std::stringstream s;

        // Set the position of the light
        s << "light[" << i << "].position";
        pProgram->setUniform(s.str(), position);

        // Set ambient light intensity
        if (L.size() > 0)
        {
            s.str("");
            s << "light[" << i << "].L";
            pProgram->setUniform(s.str(), L[0]);

            s.str("");
            s << "light[" << i << "].La";
            pProgram->setUniform(s.str(), L[0]);
        }

        // Set diffuse light intensity
        if (L.size() > 1)
        {
            s.str("");
            s << "light[" << i << "].Ld";
            pProgram->setUniform(s.str(), L[1]);
        }

        // Set specular light intensity
        if (L.size() > 2)
        {
            s.str("");
            s << "light[" << i << "].Ls";
            pProgram->setUniform(s.str(), L[2]);
        }
    }

    // Handles cursor movement events to adjust the light's position.
    void Light::onCursor(double xoffset, double yoffset, int i /*= 0*/)
    {
        const float PI05 = glm::pi<float>() / 2.0f;
        const float r = glm::sqrt(position.x * position.x +
            position.y * position.y + position.z * position.z); // Calculate radial distance
        float alpha = glm::asin(position.y / r); // Calculate elevation angle
        float betta = std::atan2f(position.x, position.z); // Calculate azimuth angle

        // Adjust angles based on cursor offsets
        alpha += yoffset > 0.0 ? -0.05f : 0.05f;
        betta += xoffset < 0.0 ? -0.05f : 0.05f;

        // Clamp alpha to avoid flipping at the poles
        alpha = glm::clamp(alpha, -PI05 + 0.01f, PI05 - 0.01f);

        // Recalculate position based on new angles
        position.x = r * glm::cos(alpha) * glm::sin(betta);
        position.y = r * glm::sin(alpha);
        position.z = r * glm::cos(alpha) * glm::cos(betta);
    }

    // Handles scroll events to zoom the light in/out by adjusting its radial distance.
    void Light::onScroll(double xoffset, double yoffset, int i /*= 0*/)
    {
        float r = glm::sqrt(position.x * position.x +
            position.y * position.y +
            position.z * position.z); // Calculate radial distance
        const float alpha = glm::asin(position.y / r); // Elevation angle
        const float betta = std::atan2f(position.x, position.z); // Azimuth angle

        // Adjust radial distance based on scroll direction
        r += yoffset > 0.0 ? -1.0f : 1.0f;
        if (r < 1.0f) r = 1.0f; // Clamp minimum radius to avoid collapsing the position

        // Recalculate position based on the new radial distance
        position.x = r * glm::cos(alpha) * glm::sin(betta);
        position.y = r * glm::sin(alpha);
        position.z = r * glm::cos(alpha) * glm::cos(betta);
    }


    LightFrustum::LightFrustum(
        const glm::vec3& center /*= { 0.0f, 10.0f, 10.0f }*/,
        const glm::vec3& at /*= { 0.0f, 0.0f, 0.0f }*/,
        const glm::vec3& up /*= { 0.0f, 1.0f, 0.0f }*/,
        float fovy /*= 45.0f*/,
        float ar /*= 1.0f*/,
        float nearDist /*= 0.5f*/,
        float farDist /*= 100.0f*/
    ) :
        center{ center },
        at{ at },
        up{ up },
        fovy{ fovy },
        ar{ ar },
        nearDist{ nearDist },
        farDist{ farDist },
        oldx{ -1.0 },
        oldy{ },
        shadowMatrix{ }
    {
        updateShadowMatrix();
    }

    void LightFrustum::onCursor(double xpos, double ypos)
    {
        if (oldx < 0.0)
        {
            oldx = xpos;
            oldy = ypos;
        }

        const float PI05 = glm::pi<float>() / 2.0f;

        const float xoffset = 0.1f * float(xpos - oldx);
        const float yoffset = 0.1f * float(oldy - ypos);

        const float r = glm::sqrt(center.x * center.x +
            center.y * center.y + center.z * center.z);
        float alpha = glm::asin(center.y / r);
        float betta = std::atan2f(center.x, center.z);

        alpha += yoffset > 0.0f ? -0.035f : 0.035f;
        betta += xoffset > 0.0f ? -0.035f : 0.035f;

        if (alpha <= -PI05)
            alpha = -PI05 + 0.01f;
        else if (alpha >= PI05)
            alpha = PI05 - 0.01f;

        center.x = r * glm::cos(alpha) * glm::sin(betta);
        center.y = r * glm::sin(alpha);
        center.z = r * glm::cos(alpha) * glm::cos(betta);

        oldx = xpos;
        oldy = ypos;

        updateShadowMatrix();
    }

    void LightFrustum::onScroll(double xoffset, double yoffset)
    {
        float r = glm::sqrt(center.x * center.x +
            center.y * center.y +
            center.z * center.z);
        const float alpha = glm::asin(center.y / r);
        const float betta = std::atan2f(center.x, center.z);

        r += yoffset > 0.0f ? -1.0f : 1.0f;
        if (r < 1.0f) r = 1.0f;

        center.x = r * glm::cos(alpha) * glm::sin(betta);
        center.y = r * glm::sin(alpha);
        center.z = r * glm::cos(alpha) * glm::cos(betta);

        updateShadowMatrix();
    }

    void LightFrustum::updateShadowMatrix()
    {
        const glm::mat4 shadowBiasMatrix = glm::mat4(
            glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),
            glm::vec4(0.0f, 0.5f, 0.0f, 0.0f),
            glm::vec4(0.0f, 0.0f, 0.5f, 0.0f),
            glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
        );

        const glm::mat4 projectionMatrix = glm::perspective(glm::radians(fovy), ar, nearDist, farDist);

        // Reference frame
        const glm::vec3 n = glm::normalize(center - at);
        const glm::vec3 u = glm::normalize(glm::cross(up, n));
        const glm::vec3 v = glm::normalize(glm::cross(n, u));

        // viewMatrix = rotationMatrix * translationMatrix;
        const glm::mat4 viewMatrix = glm::mat4(
            u.x, v.x, n.x, 0,
            u.y, v.y, n.y, 0,
            u.z, v.z, n.z, 0,
            0, 0, 0, 1
        ) *
            glm::translate(glm::mat4(1.0f), -center);

        shadowMatrix = shadowBiasMatrix * projectionMatrix * viewMatrix;
    }

}
