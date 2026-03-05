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
    Supports a blur toggle that applies Gaussian smoothing to the cartoon
    output for a soft, painterly look.
*******************************************************************************/
#version 420

layout(location=0) out vec4 FragColor;

// Texture0 = cartoon-shaded scene (from Pass 4)
layout(binding=0) uniform sampler2D Texture0;
// Texture1 = edge-detected outlines (from Pass 3)
layout(binding=1) uniform sampler2D Texture1;

// Controls the strength of edge outlines (0.0 = no edges, 1.0 = full edges)
uniform float Alpha = 1.0f;

// Mode: 1 = cartoon (composite with edges), 0 = standard (pass through shaded scene)
uniform int Mode = 1;

// Blur toggle: 1 = apply blur to cartoon output, 0 = crisp output
uniform int BlurEnabled = 1;

// Precomputed normalized Gaussian weights (same as blur passes).
float Weight[5] = float[5]( 0.158435f, 0.148836f, 0.123389f,
                            0.0902733f, 0.0582848f );

// Luminance calculation
float luminance(vec3 color)
{
    return dot(vec3(0.2126f, 0.7152f, 0.0722f), color);
}

// Apply a simple Gaussian blur to a texture at the given pixel coordinate
vec4 gaussianBlur(sampler2D tex, ivec2 pix)
{
    // Vertical component
    vec4 vSum = texelFetch(tex, pix, 0) * Weight[0];
    vSum += texelFetchOffset(tex, pix, 0, ivec2(0,-4)) * Weight[4];
    vSum += texelFetchOffset(tex, pix, 0, ivec2(0,-3)) * Weight[3];
    vSum += texelFetchOffset(tex, pix, 0, ivec2(0,-2)) * Weight[2];
    vSum += texelFetchOffset(tex, pix, 0, ivec2(0,-1)) * Weight[1];
    vSum += texelFetchOffset(tex, pix, 0, ivec2(0, 1)) * Weight[1];
    vSum += texelFetchOffset(tex, pix, 0, ivec2(0, 2)) * Weight[2];
    vSum += texelFetchOffset(tex, pix, 0, ivec2(0, 3)) * Weight[3];
    vSum += texelFetchOffset(tex, pix, 0, ivec2(0, 4)) * Weight[4];

    // Horizontal component
    vec4 hSum = texelFetch(tex, pix, 0) * Weight[0];
    hSum += texelFetchOffset(tex, pix, 0, ivec2(-4,0)) * Weight[4];
    hSum += texelFetchOffset(tex, pix, 0, ivec2(-3,0)) * Weight[3];
    hSum += texelFetchOffset(tex, pix, 0, ivec2(-2,0)) * Weight[2];
    hSum += texelFetchOffset(tex, pix, 0, ivec2(-1,0)) * Weight[1];
    hSum += texelFetchOffset(tex, pix, 0, ivec2( 1,0)) * Weight[1];
    hSum += texelFetchOffset(tex, pix, 0, ivec2( 2,0)) * Weight[2];
    hSum += texelFetchOffset(tex, pix, 0, ivec2( 3,0)) * Weight[3];
    hSum += texelFetchOffset(tex, pix, 0, ivec2( 4,0)) * Weight[4];

    return (vSum + hSum) * 0.5f;
}

void main()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);

    // Fetch cartoon-shaded color (optionally blurred)
    vec4 cartoonColor;
    if (BlurEnabled == 1)
        cartoonColor = gaussianBlur(Texture0, pix);
    else
        cartoonColor = texelFetch(Texture0, pix, 0);

    // If standard mode, just output the shaded scene (no edge compositing)
    if (Mode == 0)
    {
        FragColor = cartoonColor;
        return;
    }

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
