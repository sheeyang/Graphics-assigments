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
    // Sample the cubemap (LDR PNG - already sRGB, output directly)
    vec3 color = texture(CubeMapTex, Vec).rgb;
    FragColor = vec4(color, 1.0);
}
)"
