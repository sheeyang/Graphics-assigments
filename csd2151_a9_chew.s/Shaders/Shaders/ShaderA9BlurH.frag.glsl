R"(
/*!*****************************************************************************
\file ShaderA9BlurH.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cartoon)
\date 03/05/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 9 - Pass 3: Horizontal Gaussian Blur.
    Implements the second pass of a two-pass separable Gaussian blur filter.
    Applies a horizontal blur by sampling pixels along the X-axis using the
    same precomputed Gaussian weights as the first pass. Can be toggled
    on/off via BlurEnabled. When disabled, passes through unchanged.
*******************************************************************************/
#version 420

layout(location=0) out vec4 FragColor;

// Texture from the vertical blur pass
layout(binding=0) uniform sampler2D Texture0;
layout(binding=1) uniform sampler2D Texture1;

// Toggle blur on/off
uniform int BlurEnabled = 1;

// Precomputed normalized Gaussian weights (same as vertical pass).
float Weight[5] = float[5]( 0.158435f, 0.148836f, 0.123389f,
                            0.0902733f, 0.0582848f );

void main()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);

    if (BlurEnabled == 0)
    {
        // Pass through unchanged
        FragColor = texelFetch(Texture1, pix, 0);
        return;
    }

    // Apply horizontal Gaussian blur
    vec4 sum = texelFetch(Texture1, pix, 0) * Weight[0];

    sum += texelFetchOffset(Texture1, pix, 0, ivec2(-4, 0)) * Weight[4];
    sum += texelFetchOffset(Texture1, pix, 0, ivec2(-3, 0)) * Weight[3];
    sum += texelFetchOffset(Texture1, pix, 0, ivec2(-2, 0)) * Weight[2];
    sum += texelFetchOffset(Texture1, pix, 0, ivec2(-1, 0)) * Weight[1];
    sum += texelFetchOffset(Texture1, pix, 0, ivec2( 1, 0)) * Weight[1];
    sum += texelFetchOffset(Texture1, pix, 0, ivec2( 2, 0)) * Weight[2];
    sum += texelFetchOffset(Texture1, pix, 0, ivec2( 3, 0)) * Weight[3];
    sum += texelFetchOffset(Texture1, pix, 0, ivec2( 4, 0)) * Weight[4];

    FragColor = sum;
}
)"
