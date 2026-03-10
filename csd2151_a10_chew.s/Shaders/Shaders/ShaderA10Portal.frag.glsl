R"(
/*!*****************************************************************************
\file ShaderA10Portal.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal with Skybox)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 10 - Procedural portal texture.
    Generates a dynamic, time-based swirling portal effect inspired by
    Shadertoy. This texture is rendered to an FBO and then projected
    onto the skybox scene.
*******************************************************************************/
#version 330 core

uniform float iTime;
uniform vec2 iResolution;

layout(location=0) out vec4 FragColor;

void main()
{
    // Normalized coordinates (0 to 1) then remap to (-1 to 1)
    vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;

    // Aspect correct
    float aspect = iResolution.x / max(iResolution.y, 1.0);
    uv.x *= aspect;

    // Distance from center and angle
    float dist = length(uv);
    float angle = atan(uv.y, uv.x);

    // Swirling vortex effect
    float swirl = sin(dist * 10.0 - iTime * 3.0 + angle * 3.0) * 0.5 + 0.5;
    float ring = smoothstep(0.3, 0.32, dist) * (1.0 - smoothstep(0.9, 1.0, dist));

    // Inner glow
    float glow = exp(-dist * 2.5) * (0.8 + 0.2 * sin(iTime * 2.0));

    // Pulsating energy bands
    float bands = sin(angle * 6.0 + iTime * 4.0 - dist * 8.0) * 0.5 + 0.5;
    bands *= smoothstep(0.1, 0.4, dist) * (1.0 - smoothstep(0.7, 1.0, dist));

    // Color palette - ethereal blue/purple portal
    vec3 color1 = vec3(0.1, 0.3, 0.9);   // Deep blue
    vec3 color2 = vec3(0.6, 0.2, 0.8);   // Purple
    vec3 color3 = vec3(0.2, 0.8, 1.0);   // Cyan
    vec3 white  = vec3(1.0, 1.0, 1.0);

    vec3 col = mix(color1, color2, swirl);
    col = mix(col, color3, bands * 0.6);
    col += white * glow * 0.5;
    col *= ring + glow;

    // Outer edge fade with transparency
    float alpha = smoothstep(1.05, 0.8, dist);
    alpha = max(alpha, glow * 0.8);

    FragColor = vec4(col, alpha);
}
)"
