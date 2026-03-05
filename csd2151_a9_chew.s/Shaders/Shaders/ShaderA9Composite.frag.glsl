R"(
/*!*****************************************************************************
\file ShaderA9Composite.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cartoon)
\date 03/05/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 9 - Pass 6: Final Composite.
    Intelligently blends the edge-detected outlines with the cartoon-shaded
    geometry, producing a stylized image that combines the bold outlines of
    traditional animation with smooth cel-shaded 3D rendering. The Alpha
    factor controls the strength of edge outlines in the final image.
*******************************************************************************/
#version 420

layout(location=0) out vec4 FragColor;

// Texture0 = cartoon-shaded scene (from Pass 4)
layout(binding=0) uniform sampler2D Texture0;
// Texture1 = edge-detected outlines (from Pass 3)
layout(binding=1) uniform sampler2D Texture1;

// Controls the strength of edge outlines (0.0 = no edges, 1.0 = full edges)
uniform float Alpha = 1.0f;

// Luminance calculation
float luminance(vec3 color)
{
    return dot(vec3(0.2126f, 0.7152f, 0.0722f), color);
}

void main()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);

    // Fetch cartoon-shaded color
    vec4 cartoonColor = texelFetch(Texture0, pix, 0);

    // Fetch edge detection result (white = edge, black = no edge)
    vec4 edgeColor = texelFetch(Texture1, pix, 0);

    // Compute edge factor from luminance (1.0 where edges are detected)
    float edgeFactor = luminance(edgeColor.rgb);

    // Composite: where edges are detected, render black outlines
    // mix(cartoon, black, edgeFactor * Alpha)
    // edgeFactor=1 + Alpha=1 -> black outline
    // edgeFactor=0 -> cartoon color (no edge)
    // Alpha=0 -> cartoon color (edges disabled via alpha)
    FragColor = mix(cartoonColor, vec4(0.0f, 0.0f, 0.0f, 1.0f), edgeFactor * Alpha);
}
)"
