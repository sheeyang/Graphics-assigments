R"(
/*!*****************************************************************************
\file ShaderA1.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 2 (Ray Casting)
\date 01/17/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 1 (Tabs) - copied for use in menu subwindow.
    Implements three distinct visual effects for each tab.
*******************************************************************************/
#version 330 core

uniform vec2 iResolution;   // Viewport resolution (in pixels)
uniform float iTime;        // Time in seconds
uniform int iCurrentTab;    // Current active tab (0, 1, or 2)
uniform vec2 iMousePos;     // Mouse position in pixels

layout(location=0) out vec4 FragColor;

// Constants
const float PI = 3.14159265359;
const int NUM_TABS = 3;
const float TAB_CIRCLE_RADIUS = 15.0;
const float TAB_CIRCLE_Y_OFFSET = 30.0;
const float TAB_CIRCLE_SPACING = 40.0;

// ============================================
// Tab Indicator Drawing Functions
// ============================================

float circle(vec2 uv, vec2 center, float radius)
{
    return 1.0 - smoothstep(radius - 2.0, radius + 2.0, length(uv - center));
}

vec4 drawTabIndicators(vec2 fragCoord, int currentTab)
{
    vec4 tabColor = vec4(0.0);
    float centerX = iResolution.x / 2.0;
    float y = TAB_CIRCLE_Y_OFFSET;
    
    for (int i = 0; i < NUM_TABS; i++)
    {
        float x = centerX + (float(i) - 1.0) * TAB_CIRCLE_SPACING;
        vec2 center = vec2(x, y);
        
        if (i == currentTab)
        {
            float c = circle(fragCoord, center, TAB_CIRCLE_RADIUS);
            tabColor = max(tabColor, vec4(1.0, 1.0, 1.0, 1.0) * c);
        }
        else
        {
            float c = circle(fragCoord, center, TAB_CIRCLE_RADIUS);
            tabColor = max(tabColor, vec4(1.0, 1.0, 1.0, 0.3) * c);
        }
    }
    
    return tabColor;
}

// ============================================
// Tab 1: Animated Checkerboard
// ============================================

vec4 tab1_animatedCheckerboard(vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float speed = 0.1;
    vec2 offset = vec2(-iTime * speed, -iTime * speed);
    uv = uv + offset;
    
    float aspect = iResolution.x / iResolution.y;
    int size = 10;
    vec2 scaledUV = vec2(uv.x * aspect, uv.y) * float(size);
    float checker = mod(floor(scaledUV.x) + floor(scaledUV.y), 2.0);
    
    vec3 color1 = vec3(0.1, 0.1, 0.15);
    vec3 color2 = vec3(0.3, 0.35, 0.4);
    vec3 finalColor = mix(color1, color2, checker);
    
    return vec4(finalColor, 1.0);
}

// ============================================
// Tab 2: Blurred Radial Gradients
// ============================================

vec4 tab2_radialGradients(vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 finalColor = vec3(0.0);
    
    vec2 topRight = vec2(1.0, 1.0);
    float distTopRight = length(uv - topRight);
    
    vec2 bottomLeft = vec2(0.0, 0.0);
    float distBottomLeft = length(uv - bottomLeft);
    
    float blurRadius = 0.7;
    float gradientTopRight = 1.0 - smoothstep(0.0, blurRadius, distTopRight);
    float gradientBottomLeft = 1.0 - smoothstep(0.0, blurRadius, distBottomLeft);
    
    float timeSpeed = 1.5;
    
    vec3 colorTopRight = vec3(
        0.5 + 0.5 * sin(iTime * timeSpeed),
        0.5 + 0.5 * sin(iTime * timeSpeed + PI * 2.0 / 3.0),
        0.5 + 0.5 * sin(iTime * timeSpeed + PI * 4.0 / 3.0)
    );
    
    vec3 colorBottomLeft = vec3(
        0.5 + 0.5 * sin(iTime * timeSpeed + PI),
        0.5 + 0.5 * sin(iTime * timeSpeed + PI + PI * 2.0 / 3.0),
        0.5 + 0.5 * sin(iTime * timeSpeed + PI + PI * 4.0 / 3.0)
    );
    
    finalColor += colorTopRight * gradientTopRight * 0.8;
    finalColor += colorBottomLeft * gradientBottomLeft * 0.8;
    finalColor = clamp(finalColor, 0.0, 1.0);
    
    return vec4(finalColor, 1.0);
}

// ============================================
// Tab 3: Checkerboard with Mouse Highlight
// ============================================

vec4 tab3_checkerboardWithMouseHighlight(vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    
    float aspect = iResolution.x / iResolution.y;
    int size = 10;
    vec2 scaledUV = vec2(uv.x * aspect, uv.y) * float(size);
    float checker = mod(floor(scaledUV.x) + floor(scaledUV.y), 2.0);
    
    vec3 color1 = vec3(0.1, 0.1, 0.15);
    vec3 color2 = vec3(0.3, 0.35, 0.4);
    vec3 checkerColor = mix(color1, color2, checker);
    
    vec2 mouseUV = vec2(iMousePos.x, iResolution.y - iMousePos.y);
    float distToMouse = length(fragCoord - mouseUV);
    
    float innerRadius = 8.0;
    float outerRadius = 18.0;
    float highlight = 1.0 - smoothstep(innerRadius, outerRadius, distToMouse);
    
    vec3 highlightColor = vec3(1.0, 1.0, 1.0);
    vec3 finalColor = mix(checkerColor, highlightColor, highlight * 0.7);
    
    return vec4(finalColor, 1.0);
}

// ============================================
// Main Function
// ============================================

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec4 color;
    
    if (iCurrentTab == 0)
    {
        color = tab1_animatedCheckerboard(fragCoord);
    }
    else if (iCurrentTab == 1)
    {
        color = tab2_radialGradients(fragCoord);
    }
    else
    {
        color = tab3_checkerboardWithMouseHighlight(fragCoord);
    }
    
    vec4 tabIndicator = drawTabIndicators(fragCoord, iCurrentTab);
    color = mix(color, tabIndicator, tabIndicator.a);
    
    FragColor = color;
}
)"
