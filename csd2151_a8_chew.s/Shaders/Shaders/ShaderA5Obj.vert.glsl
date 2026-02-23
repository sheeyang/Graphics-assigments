R"(
/*!*****************************************************************************
\file ShaderA5Obj.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 5 (Environment Mapping)
\date 02/02/2026 (MM/DD/YYYY)
\brief
    Vertex shader for Assignment 5 - Object rendering with reflection and refraction.
    Computes per-vertex reflection and refraction directions in world space
    for environment mapping in the fragment shader.
*******************************************************************************/
#version 330 core

// Camera structure for world-space calculations
struct Camera 
{
    vec3 position;    // Camera position in world space
};

// Material structure for reflection/refraction properties
struct Material 
{
    vec3 color;             // Base color (unused in pure environment mapping)
    float reflectionFactor; // Blend factor between reflection and refraction
    float eta;              // Ratio of indices of refraction (n1/n2)
};

// Vertex attributes
layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec3 VertexNormal;

// Transformation matrices
uniform mat4 M; // Model transform matrix
uniform mat4 V; // View transform matrix
uniform mat4 P; // Projection transform matrix

// Uniform inputs
uniform Camera camera;
uniform Material material;

// Outputs to fragment shader
out vec3 ReflectDir;    // Reflection direction in world space
out vec3 RefractDir;    // Refraction direction in world space

void main()
{
    // Transform vertex position to world space
    vec4 positionInWorld = M * vec4(VertexPosition, 1.0f);

    // Compute normal transformation matrix (inverse transpose of model matrix)
    // This correctly handles non-uniform scaling
    mat3 N = mat3(transpose(inverse(M)));
    
    // Transform normal to world space and normalize
    vec3 normalInWorld = normalize(N * VertexNormal);

    // Compute view direction from camera to vertex (in world space)
    vec3 viewInWorld = normalize(vec3(positionInWorld) - camera.position);

    // Compute reflection direction using GLSL built-in reflect()
    // reflect(I, N) = I - 2 * dot(N, I) * N
    ReflectDir = reflect(viewInWorld, normalInWorld);
    
    // Compute refraction direction using GLSL built-in refract()
    // refract(I, N, eta) computes refracted direction using Snell's law
    // eta = n1/n2 where n1 is the index of refraction of the medium the ray is coming from
    RefractDir = refract(viewInWorld, normalInWorld, material.eta);

    // Transform vertex to clip space for rasterization
    gl_Position = P * V * positionInWorld;
}
)"
