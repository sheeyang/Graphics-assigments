R"(
/*!*****************************************************************************
\file ShaderA9CelSobel.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cel-Shading Pipeline)
\date 03/03/2026 (MM/DD/YYYY)
\brief
    Pass 4 - Sobel edge detection pass.
    This fragment shader implements edge detection using the Sobel operator
    for post-processing effects. It analyzes a rendered texture from a
    previous pass to identify edges by detecting significant luminance
    changes between neighboring pixels, generating crisp black outlines
    for the cel-shading effect.
*******************************************************************************/
#version 420

layout(location=0) out vec4 FragColor;

// Texture from the blur pass (or raw Blinn-Phong if blur is off)
layout(binding=0) uniform sampler2D Texture;

// Parameters
uniform float EdgeThreshold = 0.05f;

float luminance(vec3 color)
{
    return dot(vec3(0.2126f, 0.7152f, 0.0722f), color);
}

void main()
{
    ivec2 pix = ivec2(gl_FragCoord.xy);

    float s00 = luminance(texelFetchOffset(Texture, pix, 0, ivec2(-1, 1)).rgb);
    float s10 = luminance(texelFetchOffset(Texture, pix, 0, ivec2(-1, 0)).rgb);
    float s20 = luminance(texelFetchOffset(Texture, pix, 0, ivec2(-1,-1)).rgb);
    float s01 = luminance(texelFetchOffset(Texture, pix, 0, ivec2( 0, 1)).rgb);
    float s21 = luminance(texelFetchOffset(Texture, pix, 0, ivec2( 0,-1)).rgb);
    float s02 = luminance(texelFetchOffset(Texture, pix, 0, ivec2( 1, 1)).rgb);
    float s12 = luminance(texelFetchOffset(Texture, pix, 0, ivec2( 1, 0)).rgb);
    float s22 = luminance(texelFetchOffset(Texture, pix, 0, ivec2( 1,-1)).rgb);

    float sx = s00 + 2*s10 + s20 - (s02 + 2*s12 + s22);
    float sy = s00 + 2*s01 + s02 - (s20 + 2*s21 + s22);

    float g = sx*sx + sy*sy;

    FragColor = (g > EdgeThreshold) ? vec4(1.0f) : vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
)"
