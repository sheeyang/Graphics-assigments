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
uniform float AlphaFactor;      // Edge blending strength (0.0 - 1.0)
uniform int EdgeEnabled;        // Edge filter toggle (0=off, 1=on)

layout(location=0) out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution;

    // Sample the edge map and cartoon color
    float edgeStrength = texture(EdgeTex, uv).r;
    vec3 cartoonColor = texture(CartoonTex, uv).rgb;

    vec3 finalColor;
    if (EdgeEnabled == 1)
    {
        // Edge filter ON: apply outlines modulated by AlphaFactor
        // Where edges detected (edgeStrength=1), darken by AlphaFactor
        // Where no edges (edgeStrength=0), show cartoon color
        finalColor = cartoonColor * (1.0f - edgeStrength * AlphaFactor);
    }
    else
    {
        // Edge filter OFF: show cartoon shading only (no outlines)
        finalColor = cartoonColor;
    }

    FragColor = vec4(finalColor, 1.0f);
}
)"
