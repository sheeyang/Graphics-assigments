R"(
/*!*****************************************************************************
\file ShaderA10Sky.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal with Skybox)
\date 03/28/2026 (MM/DD/YYYY)
\brief
    Vertex shader for Assignment 10 - Pisa HDR cubebox rendering.
    Uses the full view matrix so the cubemap is rendered on a finite
    world-space box instead of an infinitely distant skybox.
*******************************************************************************/
#version 330 core

layout(location=0) in vec3 VertexPosition;

uniform mat4 V; // View transform matrix
uniform mat4 P; // Projection transform matrix

out vec3 Vec; // Direction vector for cubemap lookup

void main()
{
    Vec = VertexPosition;


    // Use the full view transform so the cubemap behaves like a finite box in world space.
    gl_Position = P * V * vec4(VertexPosition, 1.0f);
}
)"
