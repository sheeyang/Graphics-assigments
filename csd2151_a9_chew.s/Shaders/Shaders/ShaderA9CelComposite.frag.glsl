R"(
/*!*****************************************************************************
\file ShaderA9CelComposite.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cel-Shading Pipeline)
\date 03/03/2026 (MM/DD/YYYY)
\brief
    Pass 6 - Final composite pass for cel-shading pipeline.
    Intelligently blends the edge-detected outlines (from Pass 4) with the
    cartoon-shaded geometry (from Pass 5), producing a stylized image that
    combines the bold outlines of traditional animation with smooth 3D
    cel-shading.
    
    Where edges are detected, black outlines are drawn.
    Where no edges exist, the cartoon-shaded color is displayed.
*******************************************************************************/
#version 330 core

uniform sampler2D EdgeTex;      // Edge detection result (Pass 4)
uniform sampler2D CartoonTex;   // Cartoon shaded scene (Pass 5)
uniform vec2 iResolution;       // Viewport resolution

layout(location=0) out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution;

    // Sample the edge map and cartoon color
    float edgeStrength = texture(EdgeTex, uv).r;
    vec3 cartoonColor = texture(CartoonTex, uv).rgb;

    // Composite: multiply cartoon color by inverse of edge strength
    // Where edges are detected (edgeStrength = 1.0), output is black (outlines)
    // Where no edges (edgeStrength = 0.0), output is the cartoon color
    vec3 finalColor = cartoonColor * (1.0f - edgeStrength);

    FragColor = vec4(finalColor, 1.0f);
}
)"
