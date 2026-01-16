/*!*****************************************************************************
\file Demo1.frag.glsl
\author SIOW YU TENG, ETHAN (s.yutengethan@digipen.edu)
\par Course: CSD2151
\par Assignment: 2 (Ray Casting)
\date 01/16/2026 (MM/DD/YYYY)
\brief Assignment 1 Demo - Interactive multi-effect shader with UI buttons.

 * This shader implements the full Assignment 1 tab-based visual effect system
 * with circle buttons for tab selection.
 *
 * Tabs:
 *  - Tab 0: Animated diagonal checkerboard
 *  - Tab 1: Vibrant rainbow gradient interpolation  
 *  - Tab 2: Checkerboard with interactive point light (flashlight)
*******************************************************************************/

R"(
#version 330 core

uniform vec2 iResolution;               // Screen resolution in pixels
uniform float TabIndex;                 // Active visual tab index (0, 1, 2)
uniform vec2 Mouse;                     // Mouse position in screen coordinates
uniform float Time;                     // Elapsed time in seconds

layout(location=0) out vec4 FragColor;

#define PI 3.14159265359

// --- EFFECTS ---

// 1. Checkerboard
vec4 checkerboard(vec2 uv, float size) {
    uv = uv * size;
    float color = mod(floor(uv.x) + floor(uv.y), 2.0) / 5.0;
    return vec4(color, color, color, 1.0);
}

// 2. Vibrant Color Interpolation (Tab 1)
vec4 vibrantColors(vec2 responsiveUV, float responsiveScale) {
    vec2 topRightCoord = iResolution.xy / responsiveScale;
    
    float distTR = distance(responsiveUV, topRightCoord);
    float distBL = distance(responsiveUV, vec2(0.0, 0.0));
    
    float radius = 0.8;
    
    float maskTR = smoothstep(radius, 0.0, distTR); 
    float maskBL = smoothstep(radius, 0.0, distBL);
    
    float mask = maskTR + maskBL;
    
    vec3 col = 0.5 + 0.5 * cos(Time + responsiveUV.xyx + vec3(0, 2, 4));
    
    return vec4(col * mask, 1.0);
}

// 3. Point Light (Tab 2)
vec4 applyPointLight(vec4 bgInfo, vec2 fragCoord, vec2 mousePos) {
    float dist = distance(fragCoord, mousePos);
    
    float glowRadius = 150.0;
    float glowIntensity = 1.0 - smoothstep(0.0, glowRadius, dist);
    glowIntensity = glowIntensity * glowIntensity; 
    
    float ballRadius = 5.0;
    float ballIntensity = 1.0 - smoothstep(0.0, ballRadius, dist);
    
    vec3 lightColor = vec3(1.0, 0.9, 0.7);
    
    vec3 finalRGB = bgInfo.rgb;
    finalRGB += lightColor * glowIntensity * 0.8;
    finalRGB += lightColor * ballIntensity * 2.0;
    
    return vec4(finalRGB, bgInfo.a);
}

// --- CIRCLE BUTTONS ---
vec4 drawButtons(vec2 fragCoord, vec4 bg) {
    float r = 18.0; 
    float yPos = 0.1 * iResolution.y; 
    float xPos[3];
    xPos[0] = 0.50 * iResolution.x - 50.0;
    xPos[1] = 0.50 * iResolution.x;
    xPos[2] = 0.50 * iResolution.x + 50.0;
    
    vec4 result = bg; 
    
    for(int i = 0; i < 3; i++) {
        vec2 center = vec2(xPos[i], yPos);
        
        if (distance(fragCoord, center) < r) {
            bool isActive = (int(round(TabIndex)) == i);
            
            vec3 btnRGB = isActive ? vec3(1.0, 1.0, 1.0) : vec3(0.5, 0.5, 0.5);
            float alpha = 0.6; 
            
            vec3 blended = mix(result.rgb, btnRGB, alpha);
            result = vec4(blended, 1.0);
        }
    }
    return result;
}

void main()
{
    vec4 sceneColor;
    int index = int(round(TabIndex));

    float responsiveScale = (iResolution.x + iResolution.y) / 2.0;
    vec2 responsiveUV = gl_FragCoord.xy / responsiveScale;

    if (index == 1) {
        // Tab 1: Vibrant Rainbow
        sceneColor = vibrantColors(responsiveUV, responsiveScale);
    } 
    else if (index == 2) {
        // Tab 2: Static Board + Flashlight
        vec4 board = checkerboard(responsiveUV, 10.0); 
        sceneColor = applyPointLight(board, gl_FragCoord.xy, Mouse);
    } 
    else {
        // Tab 0: Diagonal Moving Checkerboard
        vec2 movingUV = responsiveUV + vec2(Time * 0.05, Time * 0.05);
        sceneColor = checkerboard(movingUV, 10.0);
    }
    
    FragColor = drawButtons(gl_FragCoord.xy, sceneColor);
}
)"
