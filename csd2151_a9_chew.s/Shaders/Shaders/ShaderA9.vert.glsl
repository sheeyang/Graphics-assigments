R"(
/*!*****************************************************************************
\file ShaderA9.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cartoon)
\date 03/05/2026 (MM/DD/YYYY)
\brief
    View-space transform vertex shader for Assignment 9 - Cartoon rendering.
    Transforms vertex positions and normals to view space for per-pixel
    lighting calculations in the fragment shader. Used by both the Blinn-Phong
    pass (for edge detection input) and the Cartoon shading pass.
*******************************************************************************/
#version 420

// Vertex attributes
layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec3 VertexNormal;

// Outputs to fragment shader
out vec3 Position;  // Vertex position in view space
out vec3 Normal;    // Vertex normal in view space

// Transformation matrices
uniform mat4 M;     // Model transform matrix
uniform mat4 V;     // View transform matrix
uniform mat4 P;     // Projection transform matrix

void main()
{
    // Compute Model-View transform matrix
    mat4 MV = V * M;

    // Normal transform matrix - inverse transpose for proper normal transformation
    mat3 N = mat3(transpose(inverse(MV)));

    // Transform normal to view space and normalize
    Normal = normalize(N * VertexNormal);

    // Transform vertex position to view space
    vec4 VertexPositionInView = MV * vec4(VertexPosition, 1.0f);
    Position = VertexPositionInView.xyz;

    // Transform vertex to clip space for rasterization
    gl_Position = P * VertexPositionInView;
}
)"
