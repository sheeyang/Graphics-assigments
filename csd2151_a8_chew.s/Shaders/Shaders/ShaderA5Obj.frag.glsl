R"(
/*!*****************************************************************************
\file ShaderA5Obj.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 5 (Environment Mapping)
\date 02/02/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 5 - Object rendering with reflection and refraction.
    Uses procedural checkerboard skybox texture for environment mapping.
    Blends reflection and refraction based on material properties.
*******************************************************************************/
#version 330 core

// Material structure for reflection/refraction properties
struct Material 
{
    vec3 color;             // Base color (unused in pure environment mapping)
    float reflectionFactor; // Blend factor: 0 = full refraction, 1 = full reflection
    float eta;              // Ratio of indices of refraction
};

// Inputs from vertex shader
in vec3 ReflectDir;     // Reflection direction in world space
in vec3 RefractDir;     // Refraction direction in world space

// Uniform inputs
uniform Material material;

// Output
layout(location=0) out vec4 FragColor;

// ============================================
// Procedural Checkerboard Texture
// ============================================
vec4 checkerboardTexture(vec2 uv, float size)
{
    uv *= size;
    float cellX = floor(uv.x);
    float cellY = floor(uv.y);
    float checker = mod(cellX + cellY, 2.0);
    return vec4(checker, checker, checker, 1.0);
}

// ============================================
// Vec2UV - Simplified Cube Mapping
// ============================================
vec2 vec2uv(vec3 v)
{
    vec3 dir = normalize(v);
    vec3 absDir = abs(dir);
    
    // Top or bottom face - return (0,0)
    if (absDir.y >= absDir.x && absDir.y >= absDir.z)
    {
        return vec2(0.0, 0.0);
    }
    
    vec2 uv;
    
    // Front or Back face (Z dominant)
    if (absDir.z >= absDir.x)
    {
        float t = 1.0 / absDir.z;
        float x = dir.x * t;
        float y = dir.y * t;
        
        if (abs(x) < 1.0 && abs(y) < 1.0)
        {
            if (dir.z < 0.0)
            {
                // Front face (z = -1)
                uv.x = (x + 1.0) * 0.5;
                uv.y = (y + 1.0) * 0.5;
            }
            else
            {
                // Back face (z = +1)
                uv.x = (-x + 1.0) * 0.5;
                uv.y = (y + 1.0) * 0.5;
            }
            return uv;
        }
    }
    
    // Left or Right face (X dominant)
    if (absDir.x > 0.0)
    {
        float t = 1.0 / absDir.x;
        float z = dir.z * t;
        float y = dir.y * t;
        
        if (abs(z) < 1.0 && abs(y) < 1.0)
        {
            if (dir.x < 0.0)
            {
                // Left face (x = -1)
                uv.x = (-z + 1.0) * 0.5;
                uv.y = (y + 1.0) * 0.5;
            }
            else
            {
                // Right face (x = +1)
                uv.x = (z + 1.0) * 0.5;
                uv.y = (y + 1.0) * 0.5;
            }
            return uv;
        }
    }
    
    return vec2(0.0, 0.0);
}

// ============================================
// Sample Procedural Skybox
// ============================================
vec3 sampleSkybox(vec3 dir)
{
    vec2 uv = vec2uv(dir);
    return checkerboardTexture(uv, 10.0).rgb;
}

void main()
{
    // Sample reflection from procedural skybox
    vec3 reflectColor = sampleSkybox(ReflectDir);
    
    // Sample refraction from procedural skybox
    // Handle total internal reflection (refract returns zero vector)
    vec3 refractColor;
    if (length(RefractDir) > 0.001)
    {
        refractColor = sampleSkybox(RefractDir);
    }
    else
    {
        // Total internal reflection - use reflection color
        refractColor = reflectColor;
    }
    
    // Blend reflection and refraction based on material property
    vec3 color = mix(refractColor, reflectColor, material.reflectionFactor);
    
    FragColor = vec4(color, 1.0);
}
)"
