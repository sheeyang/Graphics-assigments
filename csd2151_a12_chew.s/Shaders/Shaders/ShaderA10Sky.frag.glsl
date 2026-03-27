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
    vec3 hdrColor = texture(CubeMapTex, normalize(Vec)).rgb;

    // Simple Reinhard tone mapping followed by gamma correction for display.
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    vec3 color = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
)"
