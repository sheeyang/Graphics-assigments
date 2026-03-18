R"(
/*!*****************************************************************************
\file ShaderA8.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 8 (Unified Rendering)
\date 02/23/2026 (MM/DD/YYYY)
\brief
    Vertex shader for Assignment 8 - Unified scene rendering.
    Transforms vertex positions and normals to view space and passes through
    texture coordinates for PBR, cartoon, discard, and checkerboard effects.
*******************************************************************************/
#version 330 core

// Vertex attributes
layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec3 VertexNormal;
layout(location=2) in vec2 VertexTexCoord;

// Outputs to fragment shader
out vec3 Position;  // Vertex position in view space
out vec3 Normal;    // Vertex normal in view space
out vec2 TexCoord;  // Texture coordinates (pass-through)

// Transformation matrices
uniform mat4 M;     // Model transform matrix
uniform mat4 V;     // View transform matrix
uniform mat4 P;     // Projection transform matrix

void main()
{
    // Compute Model-View transform matrix
    mat4 MV = V * M;

    // Normal transform matrix - inverse transpose for proper normal transformation
    // This handles non-uniform scaling correctly
    mat3 N = mat3(transpose(inverse(MV)));

    // Transform normal to view space and normalize
    Normal = normalize(N * VertexNormal);

    // Transform vertex position to view space
    vec4 VertexPositionInView = MV * vec4(VertexPosition, 1.0f);
    Position = VertexPositionInView.xyz;

    // Pass texture coordinates through
    TexCoord = VertexTexCoord;

    // Transform vertex to clip space for rasterization
    gl_Position = P * VertexPositionInView;
}
)"
