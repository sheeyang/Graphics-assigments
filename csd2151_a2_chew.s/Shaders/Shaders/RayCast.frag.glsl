/*!*****************************************************************************
\file RayCast.frag.glsl
\author SIOW YU TENG, ETHAN (s.yutengethan@digipen.edu)
\par Course: CSD2151
\par Assignment: 2 (Ray Casting)
\date 01/16/2026 (MM/DD/YYYY)
\brief Ray Casting fragment shader implementing FPS-style 3D rendering.

 * This shader implements a classic ray casting algorithm similar to
 * Wolfenstein 3D, rendering a 2D map as a 3D first-person view.
 *
 * Features:
 *  - Wall rendering with distance-based height scaling
 *  - Distance-based brightness falloff (darker walls further away)
 *  - Solid color ceiling with optional distance shading
 *  - Floor with faded wall reflections
 *  - Multiple wall types with different visual representations
 *  - Collision-aware navigation support
 *
 * Uniform Controls:
 *  - iResolution  : Screen resolution in pixels
 *  - playerPos    : Player position in map coordinates
 *  - playerAngle  : Player viewing direction in radians
*******************************************************************************/

R"(
#version 330 core

uniform vec2 iResolution;      // Viewport resolution
uniform vec2 playerPos;        // Player position (x, y) in map coords
uniform float playerAngle;     // Player viewing angle in radians

layout(location=0) out vec4 FragColor;

#define PI 3.14159265359
#define MAP_SIZE 8
#define FOV (PI / 3.0)         // 60 degree field of view
#define MAX_DEPTH 16.0         // Maximum ray distance

// The map layout: 0 = empty, 1+ = wall types
int getMap(int x, int y) {
    // 8x8 map with different wall types
    int map[64] = int[64](
        1,1,1,1,1,1,1,1,
        1,0,0,0,0,0,0,1,
        1,0,2,0,0,3,0,1,
        1,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,1,
        1,0,3,0,0,2,0,1,
        1,0,0,0,0,0,0,1,
        1,1,1,1,1,1,1,1
    );
    
    if (x < 0 || x >= MAP_SIZE || y < 0 || y >= MAP_SIZE) return 1;
    return map[y * MAP_SIZE + x];
}

/**
 * @brief Get wall color based on wall type ID.
 * @param wallType The wall type identifier from the map.
 * @return Base RGB color for the wall.
 */
vec3 getWallColor(int wallType) {
    if (wallType == 1) {
        return vec3(0.6, 0.3, 0.3);  // Red-brown brick
    } else if (wallType == 2) {
        return vec3(0.3, 0.5, 0.7);  // Blue-gray stone
    } else if (wallType == 3) {
        return vec3(0.4, 0.6, 0.3);  // Green moss
    }
    return vec3(0.5, 0.5, 0.5);      // Default gray
}

/**
 * @brief Apply texture pattern based on wall type.
 * @param wallType The wall type identifier.
 * @param wallX The X coordinate on the wall surface (0-1).
 * @param wallY The Y coordinate on the wall surface (0-1).
 * @return Texture multiplier for color variation.
 */
float getWallTexture(int wallType, float wallX, float wallY) {
    if (wallType == 1) {
        // Brick pattern
        float brickX = fract(wallX * 4.0);
        float brickY = fract(wallY * 8.0);
        float offsetRow = floor(wallY * 8.0);
        if (mod(offsetRow, 2.0) == 1.0) brickX = fract(brickX + 0.5);
        
        float mortarX = step(0.05, brickX) * step(brickX, 0.95);
        float mortarY = step(0.1, brickY) * step(brickY, 0.9);
        return 0.7 + 0.3 * mortarX * mortarY;
    } 
    else if (wallType == 2) {
        // Stone block pattern
        float stoneX = fract(wallX * 2.0);
        float stoneY = fract(wallY * 4.0);
        float edge = step(0.03, stoneX) * step(stoneX, 0.97) * 
                     step(0.03, stoneY) * step(stoneY, 0.97);
        return 0.75 + 0.25 * edge;
    }
    else if (wallType == 3) {
        // Vertical stripes (moss/vines)
        float stripe = sin(wallX * PI * 8.0) * 0.5 + 0.5;
        float noise = sin(wallY * 20.0 + wallX * 10.0) * 0.1;
        return 0.7 + 0.3 * stripe + noise;
    }
    return 1.0;
}

/**
 * @brief Cast a single ray and return distance to wall.
 * @param rayAngle The angle of the ray in world space.
 * @param hitWallX Output: X position on wall (0-1) for texturing.
 * @param wallType Output: The type of wall hit.
 * @param isVertical Output: Whether a vertical wall was hit.
 * @return Distance to the nearest wall.
 */
float castRay(float rayAngle, out float hitWallX, out int wallType, out bool isVertical) {
    float rayDirX = cos(rayAngle);
    float rayDirY = sin(rayAngle);
    
    // Current map cell
    int mapX = int(floor(playerPos.x));
    int mapY = int(floor(playerPos.y));
    
    // Length of ray from one x/y side to next
    float deltaDistX = (rayDirX == 0.0) ? 1e30 : abs(1.0 / rayDirX);
    float deltaDistY = (rayDirY == 0.0) ? 1e30 : abs(1.0 / rayDirY);
    
    // Direction to step in x/y
    int stepX = (rayDirX < 0.0) ? -1 : 1;
    int stepY = (rayDirY < 0.0) ? -1 : 1;
    
    // Distance to next x/y boundary
    float sideDistX, sideDistY;
    if (rayDirX < 0.0)
        sideDistX = (playerPos.x - float(mapX)) * deltaDistX;
    else
        sideDistX = (float(mapX) + 1.0 - playerPos.x) * deltaDistX;
    
    if (rayDirY < 0.0)
        sideDistY = (playerPos.y - float(mapY)) * deltaDistY;
    else
        sideDistY = (float(mapY) + 1.0 - playerPos.y) * deltaDistY;
    
    // DDA algorithm
    bool hit = false;
    isVertical = false;
    float perpWallDist = 0.0;
    
    for (int i = 0; i < 64; i++) {
        if (hit) break;
        
        // Jump to next map square
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            isVertical = true;
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
            isVertical = false;
        }
        
        // Check for wall hit
        wallType = getMap(mapX, mapY);
        if (wallType > 0) hit = true;
    }
    
    // Calculate perpendicular distance (fixes fisheye)
    if (isVertical)
        perpWallDist = sideDistX - deltaDistX;
    else
        perpWallDist = sideDistY - deltaDistY;
    
    // Calculate wall X coordinate for texturing
    if (isVertical)
        hitWallX = playerPos.y + perpWallDist * rayDirY;
    else
        hitWallX = playerPos.x + perpWallDist * rayDirX;
    hitWallX = fract(hitWallX);
    
    return perpWallDist;
}

void main() {
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    float screenX = gl_FragCoord.x;
    float screenY = gl_FragCoord.y;
    
    // Calculate ray angle for this column
    float rayOffset = (screenX / iResolution.x - 0.5) * FOV;
    float rayAngle = playerAngle + rayOffset;
    
    // Cast ray
    float hitWallX;
    int wallType;
    bool isVertical;
    float distance = castRay(rayAngle, hitWallX, wallType, isVertical);
    
    // Fix fisheye distortion
    distance *= cos(rayOffset);
    
    // Calculate wall height on screen
    float wallHeight = iResolution.y / (distance + 0.0001);
    float wallTop = (iResolution.y + wallHeight) / 2.0;
    float wallBottom = (iResolution.y - wallHeight) / 2.0;
    
    vec3 color;
    
    if (screenY > wallTop) {
        // === CEILING ===
        float ceilingDist = (screenY - iResolution.y * 0.5) / (iResolution.y * 0.5);
        float ceilingShade = 1.0 - ceilingDist * 0.4;
        color = vec3(0.1, 0.1, 0.2) * ceilingShade;  // Dark blue ceiling
    }
    else if (screenY < wallBottom) {
        // === FLOOR WITH REFLECTION ===
        float floorDist = (iResolution.y * 0.5 - screenY) / (iResolution.y * 0.5);
        float floorShade = 1.0 - floorDist * 0.6;
        
        // Base floor color
        vec3 floorColor = vec3(0.15, 0.12, 0.1) * floorShade;
        
        // Add faded wall reflection
        // The reflection mirrors the wall above, so use the same ray
        float reflectionStrength = (1.0 - floorDist) * 0.3;  // Fade with distance
        
        // Get wall color for reflection
        vec3 wallCol = getWallColor(wallType);
        float brightnessFactor = 1.0 / (1.0 + distance * 0.3);
        if (isVertical) brightnessFactor *= 0.7;
        
        // Blend floor with wall reflection
        color = mix(floorColor, wallCol * brightnessFactor * 0.3, reflectionStrength);
    }
    else {
        // === WALL ===
        // Calculate wall Y coordinate for texturing
        float wallY = (screenY - wallBottom) / wallHeight;
        
        // Get base wall color
        vec3 wallCol = getWallColor(wallType);
        
        // Apply texture pattern
        float textureMult = getWallTexture(wallType, hitWallX, wallY);
        wallCol *= textureMult;
        
        // Distance-based brightness falloff
        float brightnessFactor = 1.0 / (1.0 + distance * 0.3);
        
        // Darken vertical walls for depth effect
        if (isVertical) {
            brightnessFactor *= 0.7;
        }
        
        color = wallCol * brightnessFactor;
    }
    
    // Slight vignette effect for atmosphere
    float vignette = 1.0 - length(uv - 0.5) * 0.3;
    color *= vignette;
    
    FragColor = vec4(color, 1.0);
}
)"
