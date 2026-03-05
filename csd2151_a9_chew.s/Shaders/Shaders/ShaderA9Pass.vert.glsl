R"(
/*!*****************************************************************************
\file ShaderA9Pass.vert.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cartoon)
\date 03/05/2026 (MM/DD/YYYY)
\brief
    Pass-through vertex shader for Assignment 9 - Cartoon rendering.
    Used for fullscreen quad post-processing passes (blur, edge detection,
    and final compositing). Simply passes through vertex positions.
*******************************************************************************/
#version 420

layout(location=0) in vec3 VertexPosition;

void main()
{
    gl_Position = vec4(VertexPosition.x, VertexPosition.y, 0.0f, 1.0f);
}
)"
