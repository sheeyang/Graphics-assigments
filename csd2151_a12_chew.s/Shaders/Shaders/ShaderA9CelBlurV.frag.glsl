R"(
/*!*****************************************************************************
\file ShaderA9CelBlurV.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cel-Shading Pipeline)
\date 03/03/2026 (MM/DD/YYYY)
\brief
    Pass 2 - Vertical Gaussian blur pass.
    First stage of a two-stage separable Gaussian blur filter applied to
    reduce high-frequency noise and create smoother gradients, improving
    the quality of subsequent Sobel edge detection.
    This pass applies a vertical blur by sampling pixels along the Y-axis
    using precomputed Gaussian weights.
*******************************************************************************/
#version 420

layout(location=0) out vec4 FragColor;

// Texture from the Blinn-Phong pass (Pass 0)
layout(binding=0) uniform sampler2D Texture;

// Precomputed normalized Gaussian weights.
// We need only half of all weights because they are symmetric about zero.
float Weight[5] = float[5]( 0.158435f, 0.148836f, 0.123389f, 
                             0.0902733f, 0.0582848f );

// Applies the weighted vertical sum of the Gaussian blur filter
// to store the result in yet another texture.
void main()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);
    vec4 sum = texelFetch(Texture, pix, 0);

    sum *= Weight[0];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(0,-4)) * Weight[4];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(0,-3)) * Weight[3];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(0,-2)) * Weight[2];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(0,-1)) * Weight[1];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(0, 1)) * Weight[1];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(0, 2)) * Weight[2];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(0, 3)) * Weight[3];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(0, 4)) * Weight[4];

    FragColor = sum;
}
)"
