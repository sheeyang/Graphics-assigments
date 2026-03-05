R"(
/*!*****************************************************************************
\file ShaderA5.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 5 (Environment Mapping)
\date 02/02/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 5 - Skybox with procedural checkerboard texture.
    Implements:
    - checkerboardTexture(): Procedural black/white checkerboard pattern
    - vec2uv(): Simplified cube mapping to convert 3D direction to 2D UV coords
*******************************************************************************/
#version 330 core

in vec3 Vec;

layout(location=0) out vec4 FragColor;

// ============================================
// Procedural Checkerboard Texture
// ============================================
// Implements a procedural texture mapping to apply a checkerboard-like
// pattern to a surface. The first square (corner 0,0) is black.
//
// Parameters:
//   uv   - normalized texture coordinates (0 <= uv.x < 1, 0 <= uv.y < 1)
//   size - number of black and white squares along each axis
//
// Returns:
//   vec4 color: black (0,0,0,1) or white (1,1,1,1)
vec4 checkerboardTexture(vec2 uv, float size)
{
    // Scale UV by size
    uv *= size;
    
    // Use floor to get integer cell coordinates
    // Note: Do NOT typecast, use floor() for correct negative value handling
    float cellX = floor(uv.x);
    float cellY = floor(uv.y);
    
    // Sum of coordinates determines color
    // Even sum = black, Odd sum = white
    float checker = mod(cellX + cellY, 2.0);
    
    return vec4(checker, checker, checker, 1.0);
}

// ============================================
// Vec2UV - Simplified Cube Mapping
// ============================================
// Converts a 3D direction vector to 2D UV coordinates for cube mapping.
// Only considers the 4 side faces (front, back, left, right).
// Top and bottom faces return vec2(0,0).
//
// Given:
//   - Camera at center of reference frame
//   - Normalized view vector v
//   - Axis-aligned skybox cube with corners (-1,-1,-1) and (1,1,1)
//
// Returns:
//   UV coordinates where ray intersects the skybox side
//   vec2(0,0) if ray intersects top or bottom
vec2 vec2uv(vec3 v)
{
    // Normalize the direction vector
    vec3 dir = normalize(v);
    
    // Find the dominant axis (which face we hit)
    vec3 absDir = abs(dir);
    
    // Check if we hit top or bottom first (Y dominant)
    if (absDir.y >= absDir.x && absDir.y >= absDir.z)
    {
        // Top or bottom face - return (0,0) as per spec
        return vec2(0.0, 0.0);
    }
    
    // Calculate UV coordinates based on which side face is hit
    vec2 uv;
    
    if (absDir.z >= absDir.x)
    {
        // Front (z < 0) or Back (z > 0) face
        // Calculate intersection point on z = +/-1 plane
        float t = 1.0 / absDir.z;
        float x = dir.x * t;
        float y = dir.y * t;
        
        // Check if within bounds [-1, 1)
        if (abs(x) < 1.0 && abs(y) < 1.0)
        {
            if (dir.z < 0.0)
            {
                // Front face (z = -1): looking from inside
                // UV origin at bottom-left corner (-1, -1, -1)
                uv.x = (x + 1.0) * 0.5;  // Map [-1,1] to [0,1]
                uv.y = (y + 1.0) * 0.5;
            }
            else
            {
                // Back face (z = +1): looking from inside
                // UV origin at bottom-left corner (1, -1, 1) but mirrored
                uv.x = (-x + 1.0) * 0.5;  // Mirrored
                uv.y = (y + 1.0) * 0.5;
            }
            return uv;
        }
    }
    
    // Left (x < 0) or Right (x > 0) face
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
                uv.x = (-z + 1.0) * 0.5;  // Mirrored Z
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
    
    // Fallback (should not reach here for normalized directions)
    return vec2(0.0, 0.0);
}

void main() 
{
    // Get UV coordinates from the direction vector
    vec2 uv = vec2uv(Vec);
    
    // Apply checkerboard texture with size 10 (10x10 squares per face)
    FragColor = checkerboardTexture(uv, 10.0);
}
)"
