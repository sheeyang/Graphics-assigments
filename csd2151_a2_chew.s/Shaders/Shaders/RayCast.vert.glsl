/*!*****************************************************************************
\file RayCast.vert.glsl
\author SIOW YU TENG, ETHAN (s.yutengethan@digipen.edu)
\par Course: CSD2151
\par Assignment: 2 (Ray Casting)
\date 01/16/2026 (MM/DD/YYYY)
\brief Simple pass-through vertex shader for ray casting demo.

 * This vertex shader passes through vertex positions for full-screen
 * quad rendering used by the ray casting fragment shader.
*******************************************************************************/

R"(
#version 330 core

layout(location=0) in vec3 VertexPosition;

void main() 
{
    gl_Position = vec4(VertexPosition.x, VertexPosition.y, 0.0f, 1.0f);
}
)"
