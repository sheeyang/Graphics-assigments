R"(
/*!*****************************************************************************
\file ShaderA10Portal.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal with Skybox)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 10 - Procedural wavy ring portal texture.
    Generates concentric rippling rings with color shifts, rendered to FBO.
*******************************************************************************/
#version 330 core

uniform float iTime;
uniform vec2 iResolution;

layout(location=0) out vec4 FragColor;

void main()
{
    vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;
    uv.x *= iResolution.x / max(iResolution.y, 1.0);

    float dist = length(uv);
    float angle = atan(uv.y, uv.x);

    // Wavy ring pattern: rings that wobble with angle and time
    float wave = sin(angle * 5.0 + iTime * 2.0) * 0.04;
    float rings = sin((dist + wave) * 25.0 - iTime * 4.0) * 0.5 + 0.5;

    // Second layer of rings at different frequency
    float wave2 = sin(angle * 3.0 - iTime * 1.5) * 0.03;
    float rings2 = sin((dist + wave2) * 18.0 + iTime * 3.0) * 0.5 + 0.5;

    // Ring mask: visible between inner and outer radius with wavy edges
    float edgeWave = sin(angle * 8.0 + iTime * 3.0) * 0.03;
    float innerRadius = 0.15 + edgeWave;
    float outerRadius = 0.85 + edgeWave * 0.5;

    float ringMask = smoothstep(innerRadius - 0.05, innerRadius + 0.05, dist)
                   * (1.0 - smoothstep(outerRadius - 0.05, outerRadius + 0.05, dist));

    // Core glow at center
    float coreGlow = exp(-dist * 6.0) * (0.6 + 0.4 * sin(iTime * 3.0));

    // Colors
    vec3 cyan   = vec3(0.0, 0.9, 1.0);
    vec3 blue   = vec3(0.1, 0.4, 1.0);
    vec3 purple = vec3(0.5, 0.1, 0.9);
    vec3 white  = vec3(1.0);

    // Mix rings into colour
    vec3 ringColor = mix(cyan, blue, rings);
    ringColor = mix(ringColor, purple, rings2 * 0.5);

    // Bright highlights on ring peaks
    float highlight = pow(rings, 4.0) * 0.6;
    ringColor += white * highlight;

    vec3 col = ringColor * ringMask + white * coreGlow * 0.4;

    // Outer glow halo
    float halo = (1.0 - smoothstep(0.7, 1.1, dist)) * 0.15;
    col += cyan * halo;

    float alpha = ringMask + coreGlow * 0.8 + halo;
    alpha = clamp(alpha, 0.0, 1.0);
    // Fade out at very edge
    alpha *= smoothstep(1.05, 0.85, dist);

    FragColor = vec4(col, alpha);
}
)"
