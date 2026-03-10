R"(
/*!*****************************************************************************
\file ShaderA10Sky.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Vertex shader for Assignment 10 - HDR skybox with projected portal texture.
    Strips translation from view matrix so skybox rotates with camera but does
    not translate. Computes projected texture coordinates for portal overlay.
*******************************************************************************/
#version 330 core

layout(location=0) in vec3 VertexPosition;

uniform mat4 V; // View transform matrix
uniform mat4 P; // Projection transform matrix

uniform mat4 ProjectorMatrix; // Projector matrix for projected portal texture

out vec3 Vec;              // Direction vector for cubemap lookup
out vec4 ProjTexCoord;     // Projected texture coordinates for portal

void main()
{
    // Pass vertex position as direction vector for cubemap sampling
    Vec = VertexPosition;

    // Strip translation from view matrix (keep only rotation)
    // This makes the skybox rotate with the camera but not translate
    mat4 rotView = mat4(mat3(V));

    gl_Position = P * rotView * vec4(VertexPosition, 1.0f);

    // Compute projected texture coordinates for the portal
    ProjTexCoord = ProjectorMatrix * vec4(VertexPosition, 1.0f);
}
)"
