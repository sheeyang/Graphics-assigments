R"(
/*!*****************************************************************************
\file ShaderA9Edge.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cartoon)
\date 03/05/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 9 - Pass 4: Sobel Edge Detection.
    Analyzes luminance gradients to identify object boundaries and surface
    discontinuities, generating crisp black-on-white outlines. Uses the
    Sobel operator to compute horizontal and vertical gradient magnitudes.
    Can be toggled on/off via EdgeEnabled. When disabled, outputs black
    (no edges detected).
*******************************************************************************/
#version 420

layout(location=0) out vec4 FragColor;

// Texture from the blurred Blinn-Phong pass
layout(binding=0) uniform sampler2D Texture0;
layout(binding=1) uniform sampler2D Texture1;

// Parameters
uniform float EdgeThreshold = 0.05f;
uniform int EdgeEnabled = 1;

// Luminance calculation using standard coefficients
float luminance(vec3 color)
{
    return dot(vec3(0.2126f, 0.7152f, 0.0722f), color);
}

void main()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);

    if (EdgeEnabled == 0)
    {
        // No edge detection - output black (no edges)
        FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        return;
    }

    // Sample 3x3 neighborhood luminance values
    float s00 = luminance(texelFetchOffset(Texture0, pix, 0, ivec2(-1,  1)).rgb);
    float s10 = luminance(texelFetchOffset(Texture0, pix, 0, ivec2(-1,  0)).rgb);
    float s20 = luminance(texelFetchOffset(Texture0, pix, 0, ivec2(-1, -1)).rgb);
    float s01 = luminance(texelFetchOffset(Texture0, pix, 0, ivec2( 0,  1)).rgb);
    float s21 = luminance(texelFetchOffset(Texture0, pix, 0, ivec2( 0, -1)).rgb);
    float s02 = luminance(texelFetchOffset(Texture0, pix, 0, ivec2( 1,  1)).rgb);
    float s12 = luminance(texelFetchOffset(Texture0, pix, 0, ivec2( 1,  0)).rgb);
    float s22 = luminance(texelFetchOffset(Texture0, pix, 0, ivec2( 1, -1)).rgb);

    // Sobel operator: horizontal and vertical gradients
    float sx = s00 + 2*s10 + s20 - (s02 + 2*s12 + s22);
    float sy = s00 + 2*s01 + s02 - (s20 + 2*s21 + s22);

    // Gradient magnitude (squared for efficiency)
    float g = sx*sx + sy*sy;

    // Threshold to produce crisp edges: white = edge, black = no edge
    FragColor = (g > EdgeThreshold) ? vec4(1.0f) : vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
)"
