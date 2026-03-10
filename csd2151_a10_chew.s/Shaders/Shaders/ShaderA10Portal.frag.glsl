R"(
/*!*****************************************************************************
\file ShaderA10Portal.frag.glsl
\author GitHub Copilot
\par Course: CSD2151
\par Assignment: 10 (Portal Projection)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Off-screen portal texture shader. Generates a dynamic, time-based portal
    pattern using iTime and leaves transparent pixels outside the portal mask.
*******************************************************************************/
#version 420

layout(location=0) out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1f, 311.7f))) * 43758.5453123f);
}

void main()
{
    vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0f - 1.0f;
    uv.x *= iResolution.x / max(iResolution.y, 1.0f);

    float radius = length(uv);
    float angle = atan(uv.y, uv.x);

    float wave = sin(radius * 16.0f - iTime * 5.0f + angle * 6.0f);
    float secondary = cos(radius * 24.0f + iTime * 3.0f - angle * 4.0f);
    float sparkle = hash(floor((uv + iTime * 0.1f) * 18.0f));

    vec3 innerColor = vec3(0.12f, 0.68f, 0.98f);
    vec3 outerColor = vec3(0.75f, 0.18f, 0.98f);
    vec3 glowColor = mix(innerColor, outerColor, 0.5f + 0.5f * secondary);

    float core = smoothstep(0.90f, 0.10f, radius);
    float ring = smoothstep(0.95f, 0.55f, radius) * (1.0f - smoothstep(0.55f, 0.18f, radius));
    float rim = smoothstep(0.98f, 0.78f, radius);
    float turbulence = 0.55f + 0.45f * wave;
    float stars = step(0.94f, sparkle) * core;

    vec3 color = mix(outerColor, innerColor, core);
    color += glowColor * ring * turbulence;
    color += vec3(0.9f, 0.95f, 1.0f) * stars;
    color += vec3(0.35f, 0.55f, 1.0f) * rim * 0.9f;

    float alpha = smoothstep(1.0f, 0.86f, radius);
    alpha *= 1.0f - smoothstep(0.02f, 0.14f, radius * (0.7f + 0.3f * secondary));
    alpha = max(alpha, ring * 0.9f);
    alpha = clamp(alpha, 0.0f, 1.0f);

    FragColor = vec4(color, alpha);
}
)"