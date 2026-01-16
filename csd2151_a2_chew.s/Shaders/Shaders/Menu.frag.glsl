/*!*****************************************************************************
\file Menu.frag.glsl
\author SIOW YU TENG, ETHAN (s.yutengethan@digipen.edu)
\par Course: CSD2151
\par Assignment: 2 (Ray Casting)
\date 01/16/2026 (MM/DD/YYYY)
\brief Main menu background shader with animated checkerboard.

 * This shader renders the main menu background using the animated
 * diagonal checkerboard effect from Assignment 1 Tab 0.
 * Demo thumbnails are rendered as separate interactive child windows.
*******************************************************************************/

R"(
#version 330 core

uniform vec2 iResolution;
uniform float Time;

layout(location=0) out vec4 FragColor;

// Animated checkerboard background (from Assignment 1 Tab 0)
vec4 animatedCheckerboard(vec2 uv, float size) {
    // Add time-based offset for diagonal movement
    vec2 movingUV = uv + vec2(Time * 0.05, Time * 0.05);
    movingUV = movingUV * size;
    
    // Classic checkerboard pattern
    float color = mod(floor(movingUV.x) + floor(movingUV.y), 2.0) / 5.0;
    
    return vec4(color, color, color, 1.0);
}

void main() {
    // Use responsive scaling for consistent appearance
    float responsiveScale = (iResolution.x + iResolution.y) / 2.0;
    vec2 responsiveUV = gl_FragCoord.xy / responsiveScale;
    
    // Render the animated checkerboard background
    FragColor = animatedCheckerboard(responsiveUV, 10.0);
}
)"
