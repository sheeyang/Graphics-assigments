R"(
/*!*****************************************************************************
\file ShaderA8Sky.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 8 (Unified Rendering)
\date 02/23/2026 (MM/DD/YYYY)
\brief
    Vertex shader for Assignment 8 - Night sky skybox rendering.
    Strips translation from the view matrix so the skybox rotates with the
    camera but does not translate with camera movement.
*******************************************************************************/
#version 330 core

layout(location=0) in vec3 VertexPosition;

uniform mat4 V; // View transform matrix
uniform mat4 P; // Projection transform matrix

out vec3 Vec; // Direction vector for cubemap lookup

void main()
{
    // Pass vertex position as direction vector for cubemap sampling
    Vec = VertexPosition;

    // Strip translation from view matrix (keep only rotation)
    // This makes the skybox rotate with the camera but not translate
    mat4 rotView = mat4(mat3(V));

    gl_Position = P * rotView * vec4(VertexPosition, 1.0f);
}
)"
