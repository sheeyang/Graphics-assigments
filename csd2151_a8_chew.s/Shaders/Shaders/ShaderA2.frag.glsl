R"(
/*!*****************************************************************************
\file ShaderA2.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 2 (Ray Casting)
\date 01/17/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 2 - Ray Casting implementation.
    Implements a first-person shooter style view with:
    - Wall rendering based on distance
    - Distance-based brightness (fog effect)
    - Ceiling and floor rendering
    - Different wall textures based on wall ID
    - Reflection on floor
*******************************************************************************/
#version 330 core

uniform vec2 iResolution;   // Viewport resolution (in pixels)
uniform float iTime;        // Time in seconds
uniform vec2 iCameraPos;    // Camera position in map space
uniform float iCameraDir;   // Camera direction angle in radians
uniform int iMap[81];       // Map data passed from CPU (9x9 = 81)
uniform int iMapSize;       // Map size (9)

layout(location=0) out vec4 FragColor;

// Constants
const float PI = 3.14159265359;
const float FOV = PI / 3.0;  // 60 degree field of view
const float MAX_DIST = 20.0;

// Get map value at position
int getMap(int x, int y)
{
    if (x < 0 || x >= iMapSize || y < 0 || y >= iMapSize)
        return 1; // Treat out of bounds as wall
    return iMap[y * iMapSize + x];
}

// Get wall color based on wall ID
vec3 getWallColor(int wallId, bool isVertical)
{
    vec3 color;
    
    if (wallId == 1)
        color = vec3(0.6, 0.3, 0.3);  // Red brick
    else if (wallId == 2)
        color = vec3(0.3, 0.5, 0.3);  // Green moss
    else if (wallId == 3)
        color = vec3(0.3, 0.3, 0.6);  // Blue stone
    else if (wallId == 4)
        color = vec3(0.5, 0.5, 0.3);  // Yellow sand
    else
        color = vec3(0.5, 0.5, 0.5);  // Grey default
    
    // Make vertical walls slightly darker for depth
    if (isVertical)
        color *= 0.8;
    
    return color;
}

// DDA Ray casting algorithm
struct RayResult
{
    float dist;
    int wallId;
    bool isVertical;
    float wallX;  // Where on the wall the ray hit (0-1)
};

RayResult castRay(vec2 pos, float angle)
{
    RayResult result;
    result.dist = MAX_DIST;
    result.wallId = 0;
    result.isVertical = false;
    result.wallX = 0.0;
    
    vec2 dir = vec2(cos(angle), sin(angle));
    
    // Current map cell
    int mapX = int(floor(pos.x));
    int mapY = int(floor(pos.y));
    
    // Length of ray from one side to next
    vec2 deltaDist = abs(vec2(1.0) / dir);
    
    // Direction to step in x and y
    int stepX = (dir.x < 0.0) ? -1 : 1;
    int stepY = (dir.y < 0.0) ? -1 : 1;
    
    // Distance to next x and y side
    vec2 sideDist;
    if (dir.x < 0.0)
        sideDist.x = (pos.x - float(mapX)) * deltaDist.x;
    else
        sideDist.x = (float(mapX) + 1.0 - pos.x) * deltaDist.x;
    
    if (dir.y < 0.0)
        sideDist.y = (pos.y - float(mapY)) * deltaDist.y;
    else
        sideDist.y = (float(mapY) + 1.0 - pos.y) * deltaDist.y;
    
    // DDA loop
    bool hit = false;
    bool side = false; // false = x-side, true = y-side
    
    for (int i = 0; i < 64; i++)
    {
        if (sideDist.x < sideDist.y)
        {
            sideDist.x += deltaDist.x;
            mapX += stepX;
            side = false;
        }
        else
        {
            sideDist.y += deltaDist.y;
            mapY += stepY;
            side = true;
        }
        
        int wall = getMap(mapX, mapY);
        if (wall > 0)
        {
            hit = true;
            result.wallId = wall;
            result.isVertical = side;
            
            // Calculate distance
            if (!side)
                result.dist = sideDist.x - deltaDist.x;
            else
                result.dist = sideDist.y - deltaDist.y;
            
            // Calculate wall X for texturing
            if (!side)
                result.wallX = pos.y + result.dist * dir.y;
            else
                result.wallX = pos.x + result.dist * dir.x;
            result.wallX = fract(result.wallX);
            
            break;
        }
    }
    
    return result;
}

// Simple stripe pattern for walls
float getWallPattern(float wallX, int wallId)
{
    float pattern = 1.0;
    
    if (wallId == 1)
    {
        // Brick pattern
        pattern = 0.8 + 0.2 * step(0.1, mod(wallX * 4.0, 1.0));
    }
    else if (wallId == 2)
    {
        // Vertical stripes
        pattern = 0.7 + 0.3 * step(0.5, mod(wallX * 8.0, 1.0));
    }
    else if (wallId == 3)
    {
        // Horizontal gradient feel
        pattern = 0.8 + 0.2 * wallX;
    }
    else if (wallId == 4)
    {
        // Dotted pattern
        pattern = 0.9 + 0.1 * step(0.7, mod(wallX * 6.0, 1.0));
    }
    
    return pattern;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    
    // Default camera position if not set
    vec2 camPos = iCameraPos;
    if (length(camPos) < 0.01)
        camPos = vec2(4.5, 4.5);
    
    float camDir = iCameraDir;
    
    // Calculate ray angle for this column
    float rayAngle = camDir + (uv.x - 0.5) * FOV;
    
    // Cast ray
    RayResult ray = castRay(camPos, rayAngle);
    
    // Fix fisheye effect
    float correctedDist = ray.dist * cos(rayAngle - camDir);
    
    // Calculate wall height
    float wallHeight = 1.0 / correctedDist;
    wallHeight = clamp(wallHeight, 0.0, 1.0);
    
    // Wall top and bottom positions
    float wallTop = 0.5 + wallHeight * 0.5;
    float wallBottom = 0.5 - wallHeight * 0.5;
    
    vec3 color;
    
    if (uv.y > wallTop)
    {
        // Ceiling
        float ceilDist = (uv.y - 0.5) * 2.0;
        vec3 ceilColor = vec3(0.2, 0.2, 0.3);  // Dark blue-grey ceiling
        // Add distance fade
        ceilColor *= 1.0 - ceilDist * 0.5;
        color = ceilColor;
    }
    else if (uv.y < wallBottom)
    {
        // Floor
        float floorDist = (0.5 - uv.y) * 2.0;
        vec3 floorColor = vec3(0.3, 0.25, 0.2);  // Brown floor
        
        // Add distance fade
        floorColor *= 1.0 - floorDist * 0.3;
        
        // Add reflection (faded wall color)
        if (ray.wallId > 0)
        {
            float reflectionStrength = 0.15 * (1.0 - floorDist);
            vec3 wallColor = getWallColor(ray.wallId, ray.isVertical);
            float brightness = 1.0 - (correctedDist / MAX_DIST) * 0.8;
            wallColor *= brightness;
            floorColor = mix(floorColor, wallColor, reflectionStrength);
        }
        
        color = floorColor;
    }
    else
    {
        // Wall
        vec3 wallColor = getWallColor(ray.wallId, ray.isVertical);
        
        // Apply pattern
        float pattern = getWallPattern(ray.wallX, ray.wallId);
        wallColor *= pattern;
        
        // Distance-based brightness (fog)
        float brightness = 1.0 - (correctedDist / MAX_DIST) * 1.2;
        brightness = clamp(brightness, 0.05, 1.0);
        wallColor *= brightness;
        
        // Add vertical shading for depth
        float vertShade = 1.0 - abs(uv.y - 0.5) * 0.3;
        wallColor *= vertShade;
        
        color = wallColor;
    }
    
    // Add slight vignette
    float vignette = 1.0 - length(uv - 0.5) * 0.5;
    color *= vignette;
    
    FragColor = vec4(color, 1.0);
}
)"
