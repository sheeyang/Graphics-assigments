R"(
/*!*****************************************************************************
\file ShaderA10Scene.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal with Skybox)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Vertex shader for Assignment 10 - Scene rendering with projected texture.
    Transforms vertices to view/clip space and computes projected texture
    coordinates for the portal effect.
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
out vec2 TexCoord;

uniform mat4 ProjectorMatrix; // Projection matrix for the portal projector
out vec4 ProjTexCoord;        // Projected texture coordinates

void main()
{
    mat4 MV = V * M;
    mat3 N = mat3(transpose(inverse(MV)));

    Normal = normalize(N * VertexNormal);

    vec4 VertexPositionInView = MV * vec4(VertexPosition, 1.0f);
    Position = vec3(VertexPositionInView);

    TexCoord = VertexTexCoord;

    gl_Position = P * VertexPositionInView;

    ProjTexCoord = ProjectorMatrix * M * vec4(VertexPosition, 1.0f);
}
)"
