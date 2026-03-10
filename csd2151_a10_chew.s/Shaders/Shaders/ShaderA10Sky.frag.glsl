R"(
/*!*****************************************************************************
\file ShaderA10Sky.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal with Skybox)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 10 - Pisa HDR skybox rendering.
    Samples an HDR cubemap and applies tone mapping for display.
*******************************************************************************/
#version 330 core

in vec3 Vec; // Direction vector from vertex shader

uniform samplerCube CubeMapTex; // HDR cubemap texture

layout(location=0) out vec4 FragColor;

void main()
{
    // Sample the HDR cubemap
    vec3 hdrColor = texture(CubeMapTex, Vec).rgb;

    // Simple Reinhard tone mapping for HDR -> LDR
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}
)"
