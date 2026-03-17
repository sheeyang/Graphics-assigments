R"(
/*!*****************************************************************************
\file ShaderA8Sky.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 8 (Unified Rendering)
\date 02/23/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 8 - Night sky skybox rendering.
    Samples a cubemap texture and applies fog at the horizon to soften
    the transition between sky and ground plane.
*******************************************************************************/
#version 330 core

in vec3 Vec; // Direction vector from vertex shader

uniform samplerCube CubeMapTex; // Night sky cubemap texture

// Fog parameters (shared with scene shader for consistent horizon blending)
struct Fog
{
    float minDist;
    float maxDist;
    vec4 color;
};
uniform Fog fog;

layout(location=0) out vec4 FragColor;

void main()
{
    // Sample the cubemap using the direction vector
    vec3 skyColor = texture(CubeMapTex, Vec).rgb;

    // Apply fog near the horizon to hide the sky-ground seam
    // Fragments near y=0 (horizon) get blended toward fog color
    vec3 dir = normalize(Vec);
    float horizonFactor = 1.0f - smoothstep(0.0f, 0.15f, abs(dir.y));

    // Blend sky color toward fog color at the horizon
    vec3 color = mix(skyColor, fog.color.rgb, horizonFactor * 0.8f);

    FragColor = vec4(color, 1.0f);
}
)"
