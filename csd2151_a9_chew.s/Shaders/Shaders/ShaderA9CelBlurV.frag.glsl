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
    This pass blurs along the vertical (Y) axis.
*******************************************************************************/
#version 330 core

uniform sampler2D InputTex;   // Input texture from previous pass
uniform vec2 iResolution;     // Viewport resolution

layout(location=0) out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution;
    float pixelSize = 1.0f / iResolution.y;

    // 9-tap Gaussian kernel weights (sigma ~ 1.5)
    // Weights sum to 1.0 for energy conservation
    float weights[5] = float[](0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);

    // Center sample
    vec3 result = texture(InputTex, uv).rgb * weights[0];

    // Vertical taps (up and down)
    for (int i = 1; i < 5; ++i)
    {
        float offset = float(i) * pixelSize;
        result += texture(InputTex, uv + vec2(0.0f, offset)).rgb * weights[i];
        result += texture(InputTex, uv - vec2(0.0f, offset)).rgb * weights[i];
    }

    FragColor = vec4(result, 1.0f);
}
)"
