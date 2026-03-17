R"(
/*!*****************************************************************************
\file ShaderA10Sky.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal with Skybox)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Vertex shader for Assignment 10 - Pisa HDR skybox rendering.
    Strips translation from the view matrix so the skybox rotates with
    the camera but does not translate.
*******************************************************************************/
#version 330 core

layout(location=0) in vec3 VertexPosition;

uniform mat4 V; // View transform matrix
uniform mat4 P; // Projection transform matrix

out vec3 Vec; // Direction vector for cubemap lookup

void main()
{
    Vec = VertexPosition;

    // Strip translation from view matrix (keep only rotation)
    mat4 rotView = mat4(mat3(V));

    vec4 pos = P * rotView * vec4(VertexPosition, 1.0f);
    gl_Position = pos;
}
)"
