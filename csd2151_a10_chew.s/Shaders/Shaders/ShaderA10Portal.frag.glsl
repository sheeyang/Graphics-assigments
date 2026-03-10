R"(
/*!*****************************************************************************
\file ShaderA10Portal.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 10 - Procedural portal texture generation.
    Generates a dynamic, time-based swirling portal texture inspired by
    Shadertoy techniques. Uses iTime for animation and iResolution for
    aspect-correct UV coordinates.

    The portal features:
    - Swirling vortex pattern with concentric rings
    - Color cycling through blue/purple/cyan palette
    - Glowing edge effect that pulses over time
    - Transparent edges for blending with the scene
*******************************************************************************/
#version 330 core

uniform float iTime;        // Time in seconds (set automatically by framework)
uniform vec2 iResolution;   // Viewport resolution in pixels

layout(location=0) out vec4 FragColor;

// Smooth noise function for organic patterns
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Fractal Brownian Motion for complex patterns
float fbm(vec2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 5; i++)
    {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main()
{
    // Normalized coordinates centered at (0,0)
    vec2 uv = (gl_FragCoord.xy - 0.5 * iResolution.xy) / min(iResolution.x, iResolution.y);

    // Distance from center and angle
    float dist = length(uv);
    float angle = atan(uv.y, uv.x);

    // Swirling vortex distortion
    float swirl = iTime * 1.5;
    float swirlAmount = 3.0 / (dist + 0.5);
    angle += swirlAmount + swirl;

    // Create swirling coordinates
    vec2 swirlUV = vec2(cos(angle), sin(angle)) * dist;

    // Concentric rings with time-based animation
    float rings = sin(dist * 15.0 - iTime * 3.0) * 0.5 + 0.5;
    rings *= smoothstep(0.5, 0.0, dist);

    // Spiral arms
    float spiral = sin(angle * 3.0 + dist * 10.0 - iTime * 4.0) * 0.5 + 0.5;
    spiral *= smoothstep(0.5, 0.1, dist);

    // FBM noise for organic distortion
    float n = fbm(swirlUV * 3.0 + iTime * 0.5);

    // Combine patterns
    float pattern = rings * 0.5 + spiral * 0.3 + n * 0.4;

    // Portal color palette - blue/purple/cyan
    vec3 color1 = vec3(0.1, 0.2, 0.8);   // Deep blue
    vec3 color2 = vec3(0.5, 0.1, 0.9);   // Purple
    vec3 color3 = vec3(0.1, 0.8, 0.9);   // Cyan

    // Color mixing based on pattern
    vec3 color = mix(color1, color2, pattern);
    color = mix(color, color3, rings * spiral);

    // Bright core glow
    float glow = exp(-dist * 4.0);
    color += vec3(0.5, 0.7, 1.0) * glow * (0.8 + 0.2 * sin(iTime * 2.0));

    // Energy pulses from center
    float pulse = sin(dist * 20.0 - iTime * 5.0) * 0.5 + 0.5;
    pulse *= exp(-dist * 3.0);
    color += vec3(0.3, 0.5, 1.0) * pulse * 0.5;

    // Edge glow ring
    float edgeGlow = smoothstep(0.48, 0.45, dist) * smoothstep(0.3, 0.35, dist);
    edgeGlow *= 0.5 + 0.5 * sin(iTime * 3.0 + angle * 5.0);
    color += vec3(0.2, 0.4, 1.0) * edgeGlow * 1.5;

    // Alpha: solid center, fade at edges
    float alpha = smoothstep(0.5, 0.2, dist);
    alpha = max(alpha, glow * 0.8);

    // Clamp brightness
    color = clamp(color, 0.0, 1.5);

    FragColor = vec4(color, alpha);
}
)"
