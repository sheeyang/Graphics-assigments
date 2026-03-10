R"(
/*!*****************************************************************************
\file ShaderA10Portal.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 10 - Procedural portal texture generation.
    Creates a dynamic, time-based swirling vortex pattern inspired by Shadertoy.
    Uses iTime for animation and outputs RGBA with alpha for blending control.
*******************************************************************************/
#version 330 core

uniform float iTime;
uniform vec2 iResolution;

layout(location=0) out vec4 FragColor;

void main()
{
    // Normalize coordinates to [-1, 1] with aspect correction
    vec2 uv = (gl_FragCoord.xy - 0.5 * iResolution.xy) / min(iResolution.x, iResolution.y);

    // Convert to polar coordinates
    float r = length(uv);
    float angle = atan(uv.y, uv.x);

    // Swirling vortex: rotate angle based on distance and time
    float swirl = 6.0 * r - iTime * 3.0;
    float spiral = sin(angle * 5.0 + swirl) * 0.5 + 0.5;

    // Pulsing rings
    float rings = sin(r * 20.0 - iTime * 4.0) * 0.5 + 0.5;

    // Combine patterns
    float pattern = mix(spiral, rings, 0.5);

    // Color gradient: cyan to magenta to white at center
    vec3 col1 = vec3(0.1, 0.6, 0.9);  // Cyan-blue
    vec3 col2 = vec3(0.8, 0.2, 0.8);  // Magenta
    vec3 col3 = vec3(1.0, 1.0, 1.0);  // White (center glow)

    vec3 color = mix(col1, col2, pattern);

    // Center glow effect
    float glow = exp(-r * 2.5);
    color = mix(color, col3, glow * 0.7);

    // Brightness boost near center
    color *= 1.0 + glow * 0.5;

    // Edge fade - smooth circular boundary
    float edgeFade = 1.0 - smoothstep(0.6, 1.0, r);

    // Alpha: fully opaque in center, fades at edges
    float alpha = edgeFade * (0.7 + 0.3 * glow);

    FragColor = vec4(color * edgeFade, alpha);
}
)"
