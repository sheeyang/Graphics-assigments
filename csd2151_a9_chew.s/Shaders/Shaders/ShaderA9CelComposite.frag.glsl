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
#version 420

layout(location=0) out vec4 FragColor;

// Edge detection result (Pass 3 - Sobel)
layout(binding=0) uniform sampler2D EdgeTex;

// Cartoon shaded scene (Pass 4 - Cartoon)
layout(binding=1) uniform sampler2D CartoonTex;

// Parameters
uniform float AlphaFactor = 1.0f;   // Edge blending strength (0.0 - 1.0)
uniform int EdgeEnabled = 1;        // Edge filter toggle (0=off, 1=on)

void main()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);

    // Sample the edge map and cartoon color using texelFetch
    float edgeStrength = texelFetch(EdgeTex, pix, 0).r;
    vec3 cartoonColor = texelFetch(CartoonTex, pix, 0).rgb;

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
