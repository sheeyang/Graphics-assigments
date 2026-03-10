R"(
/*!*****************************************************************************
\file ShaderA10Scene.vert.glsl
\author GitHub Copilot
\par Course: CSD2151
\par Assignment: 10 (Portal Projection)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Scene vertex shader for Assignment 10. Produces both the standard view-space
    data needed for lighting and the projected texture coordinates used to blend
    the portal texture into the scene.
*******************************************************************************/
#version 330 core

layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec3 VertexNormal;
layout(location=2) in vec2 VertexTexCoord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 ProjectorMatrix;

out vec3 Position;
out vec3 Normal;
out vec3 WorldPosition;
out vec3 WorldNormal;
out vec2 TexCoord;
out vec4 ProjTexCoord;

void main()
{
    vec4 worldPosition = M * vec4(VertexPosition, 1.0f);
    mat4 MV = V * M;

    Position = vec3(MV * vec4(VertexPosition, 1.0f));
    Normal = normalize(mat3(transpose(inverse(MV))) * VertexNormal);
    WorldPosition = worldPosition.xyz;
    WorldNormal = normalize(mat3(transpose(inverse(M))) * VertexNormal);
    TexCoord = VertexTexCoord;
    ProjTexCoord = ProjectorMatrix * worldPosition;

    gl_Position = P * MV * vec4(VertexPosition, 1.0f);
}
)"