R"(
/*!*****************************************************************************
\file Shader.frag.glsl
\author Vadim Surov (vsurov@digipen.edu)
\co-author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 1 (Tabs)
\date 01/11/2026 (MM/DD/YYYY)
\brief
    Fragment shader for the Tabs assignment. Implements three distinct visual
    effects for each tab and renders tab indicators at the bottom of the screen.
    
    Tab Effects:
    - Tab 1: Animated checkerboard pattern with square cells moving down-left
    - Tab 2: Blurred radial gradients at corners with sine-wave color cycling
    - Tab 3: Static checkerboard with white blurred highlight following cursor
    
    Tab Indicators:
    - Active tab: Full opacity white circle
    - Inactive tabs: Low opacity white circles
    - Positioned at bottom center of screen
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

/*!
 * \brief Creates a smooth circular shape with anti-aliasing
 * \param uv Current fragment coordinate in screen space
 * \param center Center position of the circle in screen space
 * \param radius Radius of the circle in pixels
 * \return Float value from 0.0 (outside) to 1.0 (inside) with smooth edges
 */
float circle(vec2 uv, vec2 center, float radius)
{
    return 1.0 - smoothstep(radius - 2.0, radius + 2.0, length(uv - center));
}

/*!
 * \brief Renders tab indicator circles at the bottom center of the screen
 * \param fragCoord Current fragment coordinate in screen space
 * \param currentTab Index of the currently active tab (0-2)
 * \return RGBA color with active tab at full opacity, inactive tabs at 30% opacity
 */
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
            // Filled circle for active tab
            float c = circle(fragCoord, center, TAB_CIRCLE_RADIUS);
            tabColor = max(tabColor, vec4(1.0, 1.0, 1.0, 1.0) * c);
        }
        else
        {
            // Filled circle for inactive tabs with lower opacity
            float c = circle(fragCoord, center, TAB_CIRCLE_RADIUS);
            tabColor = max(tabColor, vec4(1.0, 1.0, 1.0, 0.3) * c);
        }
    }
    
    return tabColor;
}

// ============================================
// Tab 1: Animated Checkerboard (moves down-left)
// ============================================

/*!
 * \brief Renders an animated checkerboard pattern moving diagonally down-left
 * \param fragCoord Current fragment coordinate in screen space
 * \return RGBA color with alternating dark/light squares in continuous motion
 * 
 * Animation speed: 0.1 units per second
 * Checkerboard size: 10x10 cells (adjusted for aspect ratio)
 */
vec4 tab1_animatedCheckerboard(vec2 fragCoord)
{
    // Calculate UV with animation offset (moving down-left)
    vec2 uv = fragCoord / iResolution.xy;
    
    // Speed of movement
    float speed = 0.1;
    
    // Offset: down is negative Y in UV, left is negative X
    vec2 offset = vec2(-iTime * speed, -iTime * speed);
    
    uv = uv + offset;
    
    // Checkerboard pattern with square cells
    float aspect = iResolution.x / iResolution.y;
    int size = 10;
    vec2 scaledUV = vec2(uv.x * aspect, uv.y) * float(size);
    float checker = mod(floor(scaledUV.x) + floor(scaledUV.y), 2.0);
    
    // Create a gradient color based on checker
    vec3 color1 = vec3(0.1, 0.1, 0.15);  // Dark color
    vec3 color2 = vec3(0.3, 0.35, 0.4);  // Light color
    
    vec3 finalColor = mix(color1, color2, checker);
    
    return vec4(finalColor, 1.0);
}

// ============================================
// Tab 2: Blurred Radial Gradients with Sine Wave Colors
// ============================================

/*!
 * \brief Renders blurred radial gradients from corners with time-based color cycling
 * \param fragCoord Current fragment coordinate in screen space
 * \return RGBA color with overlapping gradients from top-right and bottom-left
 * 
 * Gradients emanate from top-right and bottom-left corners
 * Colors cycle using sine waves with 1.5 second period
 * Phase offset between gradients creates dynamic color interplay
 */
vec4 tab2_radialGradients(vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    
    // Black background
    vec3 finalColor = vec3(0.0);
    
    // Top-right corner gradient position
    vec2 topRight = vec2(1.0, 1.0);
    float distTopRight = length(uv - topRight);
    
    // Bottom-left corner gradient position
    vec2 bottomLeft = vec2(0.0, 0.0);
    float distBottomLeft = length(uv - bottomLeft);
    
    // Blur radius (larger = more blurred)
    float blurRadius = 0.7;
    
    // Create blurred gradients
    float gradientTopRight = 1.0 - smoothstep(0.0, blurRadius, distTopRight);
    float gradientBottomLeft = 1.0 - smoothstep(0.0, blurRadius, distBottomLeft);
    
    // Sine wave color cycling
    float timeSpeed = 1.5;
    
    // Top-right gradient colors (cycling through warm colors)
    vec3 colorTopRight = vec3(
        0.5 + 0.5 * sin(iTime * timeSpeed),
        0.5 + 0.5 * sin(iTime * timeSpeed + PI * 2.0 / 3.0),
        0.5 + 0.5 * sin(iTime * timeSpeed + PI * 4.0 / 3.0)
    );
    
    // Bottom-left gradient colors (cycling with phase offset)
    vec3 colorBottomLeft = vec3(
        0.5 + 0.5 * sin(iTime * timeSpeed + PI),
        0.5 + 0.5 * sin(iTime * timeSpeed + PI + PI * 2.0 / 3.0),
        0.5 + 0.5 * sin(iTime * timeSpeed + PI + PI * 4.0 / 3.0)
    );
    
    // Combine gradients
    finalColor += colorTopRight * gradientTopRight * 0.8;
    finalColor += colorBottomLeft * gradientBottomLeft * 0.8;
    
    // Clamp to prevent overbrightening
    finalColor = clamp(finalColor, 0.0, 1.0);
    
    return vec4(finalColor, 1.0);
}

// ============================================
// Tab 3: Checkerboard with Blurred Mouse Highlight
// ============================================

/*!
 * \brief Renders a static checkerboard with a white blurred highlight at mouse position
 * \param fragCoord Current fragment coordinate in screen space
 * \return RGBA color with checkerboard base and white highlight following cursor
 * 
 * Checkerboard size: 10x10 cells (adjusted for aspect ratio)
 * Highlight: Circular gradient with 8px inner radius, 18px outer radius
 * Highlight intensity: 70% blend with white color
 */
vec4 tab3_checkerboardWithMouseHighlight(vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    
    // Static checkerboard pattern with square cells
    float aspect = iResolution.x / iResolution.y;
    int size = 10;
    vec2 scaledUV = vec2(uv.x * aspect, uv.y) * float(size);
    float checker = mod(floor(scaledUV.x) + floor(scaledUV.y), 2.0);
    
    vec3 color1 = vec3(0.1, 0.1, 0.15);  // Dark color
    vec3 color2 = vec3(0.3, 0.35, 0.4);  // Light color
    
    vec3 checkerColor = mix(color1, color2, checker);
    
    // Mouse highlight - convert mouse position to same coordinate system
    // Note: iMousePos is in window coordinates (origin top-left)
    // fragCoord is in OpenGL coordinates (origin bottom-left)
    vec2 mouseUV = vec2(iMousePos.x, iResolution.y - iMousePos.y);
    
    // Distance from fragment to mouse position
    float distToMouse = length(fragCoord - mouseUV);
    
    // Blurred highlight around cursor - sharp inner circle, then blur outward
    float innerRadius = 8.0;
    float outerRadius = 18.0;
    float highlight = 1.0 - smoothstep(innerRadius, outerRadius, distToMouse);
    
    // Highlight color (white)
    vec3 highlightColor = vec3(1.0, 1.0, 1.0);
    
    // Blend highlight with checkerboard
    vec3 finalColor = mix(checkerColor, highlightColor, highlight * 0.7);
    
    return vec4(finalColor, 1.0);
}

// ============================================
// Main Function
// ============================================

/*!
 * \brief Fragment shader entry point - selects and renders current tab content
 * 
 * Renders the appropriate tab content based on iCurrentTab uniform:
 * - Tab 0: Animated checkerboard with diagonal movement
 * - Tab 1: Blurred radial gradients with color cycling
 * - Tab 2: Static checkerboard with mouse-following highlight
 * 
 * Tab indicator circles are composited on top of all tab content
 */
void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec4 color;
    
    // Select tab content based on current tab
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
    
    // Draw tab indicators on top
    vec4 tabIndicator = drawTabIndicators(fragCoord, iCurrentTab);
    
    // Blend tab indicators over the content
    color = mix(color, tabIndicator, tabIndicator.a);
    
    FragColor = color;
}
)"