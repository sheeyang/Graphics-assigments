R"(
/*!*****************************************************************************
\file Shader.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 2 (Ray Casting)
\date 01/17/2026 (MM/DD/YYYY)
\brief
    Default fragment shader - displays checkerboard pattern.
    This file is kept for compatibility but the actual shaders used are:
    - ShaderMenu.frag.glsl (menu background)
    - ShaderA1.frag.glsl (Assignment 1 tabs)
    - ShaderA2.frag.glsl (Assignment 2 ray casting)
*******************************************************************************/
#version 330 core

uniform vec2 iResolution;   // Viewport resolution (in pixels)

layout(location=0) out vec4 FragColor;

vec4 checkerboardTexture(vec2 uv, int size)
{
    uv = uv * size;
    float color = mod(floor(uv.x) + floor(uv.y), 2.0f) / 5.0f;
    return vec4(color, color, color, 1.0f);
}

void main()
{
    FragColor = checkerboardTexture(gl_FragCoord.xy / iResolution.xy, 10);
}
)"