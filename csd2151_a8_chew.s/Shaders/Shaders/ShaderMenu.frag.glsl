R"(
/*!*****************************************************************************
\file ShaderMenu.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 2 (Ray Casting)
\date 01/17/2026 (MM/DD/YYYY)
\brief
    Fragment shader for the main menu background.
    Uses the animated checkerboard pattern from Assignment 1 Tab 1.
*******************************************************************************/
#version 330 core

uniform vec2 iResolution;   // Viewport resolution (in pixels)
uniform float iTime;        // Time in seconds
uniform int iNumSubwindows; // Number of subwindows
uniform int iGridSize;      // Grid size (4x4)

layout(location=0) out vec4 FragColor;

// Constants
const float PI = 3.14159265359;

// ============================================
// Animated Checkerboard (from Assignment 1 Tab 1)
// ============================================

vec4 animatedCheckerboard(vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    
    // Speed of movement
    float speed = 0.05;
    
    // Offset: moving down-left
    vec2 offset = vec2(-iTime * speed, -iTime * speed);
    uv = uv + offset;
    
    // Checkerboard pattern with square cells
    float aspect = iResolution.x / iResolution.y;
    int size = 15;  // Larger grid for menu background
    vec2 scaledUV = vec2(uv.x * aspect, uv.y) * float(size);
    float checker = mod(floor(scaledUV.x) + floor(scaledUV.y), 2.0);
    
    // Darker colors for menu background
    vec3 color1 = vec3(0.08, 0.08, 0.12);  // Very dark
    vec3 color2 = vec3(0.15, 0.15, 0.22);  // Slightly lighter
    
    vec3 finalColor = mix(color1, color2, checker);
    
    // Add subtle gradient overlay
    float gradient = 1.0 - length(uv - 0.5) * 0.3;
    finalColor *= gradient;
    
    return vec4(finalColor, 1.0);
}

// ============================================
// Main Function
// ============================================

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    
    // Render the animated checkerboard background
    vec4 color = animatedCheckerboard(fragCoord);
    
    FragColor = color;
}
)"
