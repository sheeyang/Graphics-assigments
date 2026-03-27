/*!*****************************************************************************
\file CG/camera.h
\author Vadim Surov (vsurov@digipen.edu)
\date 12/23/2025 (MM/DD/YYYY)
\brief Camera interface for the framework: types, state, and API for view/projection
       computation and input handling.
*******************************************************************************/
#pragma once

#include "program.h"

#include <glm/glm.hpp> // For mathematical types like vec2, vec3, mat4
#include <glm/ext.hpp> // For mathematical constants like pi

namespace cg
{
    // Enumeration to define the type of camera
    enum CameraType { ORBITING, WALKING };

    // Camera class to handle various camera functionalities
    class Camera
    {
        CameraType cameraType;  // Stores the type of camera (ORBITING or WALKING)
        glm::vec3 position;     // Current position of the camera in 3D space
        glm::vec3 target;       // Target point that the camera is looking at
        float fieldOfView;      // Field of view angle in degrees
        float near;             // Near clipping plane
        float far;              // Far clipping plane

        double oldx; // Previous x-coordinate for cursor handling
        double oldy; // Previous y-coordinate for cursor handling

    public:
        // Constructor to initialize a camera with custom or default values
        Camera(
            CameraType cameraType,
            glm::vec3 position = { 0.0f, 3.0f, 3.0f },
            glm::vec3 target = { 0.0f, 0.0f, 0.0f },
            float fieldOfView = 45.0f,
            float near = 0.5f,
            float far = 200.0f
        ) :
            cameraType{ cameraType },
            position{ position },
            target{ target },
            fieldOfView{ fieldOfView },
            near{ near },
            far{ far },
            oldx{ -1.0 },
            oldy{ -1.0 }
        { }

        // Overloaded constructor with default values for an orbiting camera
        Camera(
            glm::vec3 position = { 0.0f, 3.0f, 3.0f },
            float fieldOfView = 45.0f
        ) :
            cameraType{ CameraType::ORBITING },
            position{ position },
            target{ 0.0f, 0.0f, 0.0f },
            fieldOfView{ fieldOfView },
            near{ 0.5f },
            far{ 200.0f },
            oldx{ -1.0 },
            oldy{ -1.0 }
        { }

        // Sets the camera type and updates the target if it's orbiting
        void setType(CameraType cameraType);

        // Updates the shader program with camera position
        void setUniforms(Program* pProgram) const;

        // Returns the current position of the camera
        glm::vec3 getPosition() const { return position; }

        // Returns the view matrix for the camera using the "look at" method
        glm::mat4 getLookAt(glm::vec3 center = { 0.0f, 0.0f, 0.0f }, glm::vec3 up = { 0.0f, 1.0f, 0.0f }) const;

        // Returns the perspective projection matrix based on the field of view and aspect ratio
        glm::mat4 getPerspective(float aspect = 1) const;

        // Handles cursor movement to adjust camera orientation
        void onCursor(double xoffset, double yoffset);

        // Handles scroll input to adjust zoom or camera position
        void onScroll(double xoffset, double yoffset);
    };
}
