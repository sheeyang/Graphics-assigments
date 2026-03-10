R"(
/*!*****************************************************************************
\file ShaderA10Scene.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Vertex shader for Assignment 10 - Scene rendering with projected portal texture.
    Transforms vertex positions and normals to view space, passes texture
    coordinates, and calculates projected texture coordinates for the portal
    effect using the projector matrix technique.
*******************************************************************************/
#version 330 core

layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec3 VertexNormal;
layout(location=2) in vec2 VertexTexCoord;

uniform mat4 M;     // Model
uniform mat4 V;     // View
uniform mat4 P;     // Projection

out vec3 Position;  // Position in view space
out vec3 Normal;    // Normal in view space
out vec2 TexCoord;  // Texture coordinates
out vec3 WorldPos;  // Position in world space (for cubemap sampling)

uniform mat4 ProjectorMatrix; // Projection matrix for the portal projector
out vec4 ProjTexCoord;  // Texture coordinates for the projected portal texture

void main()
{
    mat4 MV = V * M; // Model-View transform matrix
    mat3 N = mat3(transpose(inverse(MV))); // Normal transform matrix

    Normal = normalize(N * VertexNormal);

    vec4 VertexPositionInView = MV * vec4(VertexPosition, 1.0f);
    Position = vec3(VertexPositionInView);

    // World position for cubemap sampling (skybox reflection blending)
    WorldPos = vec3(M * vec4(VertexPosition, 1.0f));

    TexCoord = VertexTexCoord;

    gl_Position = P * VertexPositionInView;

    // Calculate projected texture coordinates for the portal
    ProjTexCoord = ProjectorMatrix * M * vec4(VertexPosition, 1.0f);
}
)"
