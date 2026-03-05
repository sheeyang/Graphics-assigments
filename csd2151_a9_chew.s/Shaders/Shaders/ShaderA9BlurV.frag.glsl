R"(
/*!*****************************************************************************
\file ShaderA9BlurV.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cartoon)
\date 03/05/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 9 - Pass 2: Vertical Gaussian Blur.
    Implements the first pass of a two-pass separable Gaussian blur filter.
    Applies a vertical blur by sampling pixels along the Y-axis using
    precomputed Gaussian weights. Can be toggled on/off via BlurEnabled.
    When disabled, passes through the input texture unchanged.
*******************************************************************************/
#version 420

layout(location=0) out vec4 FragColor;

// Texture from the Blinn-Phong pass
layout(binding=0) uniform sampler2D Texture0;
layout(binding=1) uniform sampler2D Texture1;

// Toggle blur on/off
uniform int BlurEnabled = 1;

// Precomputed normalized Gaussian weights.
// Only half the weights are needed due to symmetry about zero.
float Weight[5] = float[5]( 0.158435f, 0.148836f, 0.123389f,
                            0.0902733f, 0.0582848f );

void main()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);

    if (BlurEnabled == 0)
    {
        // Pass through unchanged
        FragColor = texelFetch(Texture0, pix, 0);
        return;
    }

    // Apply vertical Gaussian blur
    vec4 sum = texelFetch(Texture0, pix, 0) * Weight[0];

    sum += texelFetchOffset(Texture0, pix, 0, ivec2(0, -4)) * Weight[4];
    sum += texelFetchOffset(Texture0, pix, 0, ivec2(0, -3)) * Weight[3];
    sum += texelFetchOffset(Texture0, pix, 0, ivec2(0, -2)) * Weight[2];
    sum += texelFetchOffset(Texture0, pix, 0, ivec2(0, -1)) * Weight[1];
    sum += texelFetchOffset(Texture0, pix, 0, ivec2(0,  1)) * Weight[1];
    sum += texelFetchOffset(Texture0, pix, 0, ivec2(0,  2)) * Weight[2];
    sum += texelFetchOffset(Texture0, pix, 0, ivec2(0,  3)) * Weight[3];
    sum += texelFetchOffset(Texture0, pix, 0, ivec2(0,  4)) * Weight[4];

    FragColor = sum;
}
)"
