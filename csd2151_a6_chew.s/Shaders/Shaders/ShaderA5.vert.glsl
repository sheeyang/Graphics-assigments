R"(
/*!*****************************************************************************
\file ShaderA5.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 5 (Environment Mapping)
\date 02/02/2026 (MM/DD/YYYY)
\brief
    Vertex shader for Assignment 5 - Skybox with procedural checkerboard texture.
    Passes vertex positions as direction vectors for skybox texture lookup.
*******************************************************************************/
#version 330 core

layout(location=0) in vec3 VertexPosition;

uniform mat4 M; // Model transform matrix
uniform mat4 V; // View transform matrix
uniform mat4 P; // Projection transform matrix

out vec3 Vec;

void main()
{
    // Pass vertex position as direction vector for skybox lookup
    Vec = VertexPosition; 
    gl_Position = P * V * M * vec4(VertexPosition, 1.0f);
}
)"
