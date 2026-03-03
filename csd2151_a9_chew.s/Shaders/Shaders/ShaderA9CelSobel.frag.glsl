R"(
/*!*****************************************************************************
\file ShaderA9CelSobel.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cel-Shading Pipeline)
\date 03/03/2026 (MM/DD/YYYY)
\brief
    Pass 4 - Sobel edge detection pass.
    Analyzes luminance gradients from the blurred Blinn-Phong image to
    identify object boundaries and surface discontinuities, generating
    crisp black outlines for the cel-shading effect.
    
    Uses a 3x3 Sobel operator on luminance values to compute gradient
    magnitude, then applies smoothstep thresholding for clean edges.
*******************************************************************************/
#version 330 core

uniform sampler2D InputTex;   // Input texture from horizontal blur pass
uniform vec2 iResolution;     // Viewport resolution
uniform float EdgeThreshold;  // Upper threshold for edge detection (adjustable)

layout(location=0) out vec4 FragColor;

// Convert RGB to luminance using standard coefficients
float luminance(vec3 color)
{
    return dot(color, vec3(0.2126f, 0.7152f, 0.0722f));
}

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution;
    vec2 texelSize = 1.0f / iResolution;

    // Sample 3x3 neighborhood luminance values
    //  tl  tc  tr
    //  ml  mc  mr
    //  bl  bc  br
    float tl = luminance(texture(InputTex, uv + vec2(-texelSize.x,  texelSize.y)).rgb);
    float tc = luminance(texture(InputTex, uv + vec2( 0.0f,         texelSize.y)).rgb);
    float tr = luminance(texture(InputTex, uv + vec2( texelSize.x,  texelSize.y)).rgb);

    float ml = luminance(texture(InputTex, uv + vec2(-texelSize.x,  0.0f)).rgb);
    // mc not needed for Sobel
    float mr = luminance(texture(InputTex, uv + vec2( texelSize.x,  0.0f)).rgb);

    float bl = luminance(texture(InputTex, uv + vec2(-texelSize.x, -texelSize.y)).rgb);
    float bc = luminance(texture(InputTex, uv + vec2( 0.0f,        -texelSize.y)).rgb);
    float br = luminance(texture(InputTex, uv + vec2( texelSize.x, -texelSize.y)).rgb);

    // Sobel X kernel:  [-1  0  1]
    //                   [-2  0  2]
    //                   [-1  0  1]
    float Gx = -tl - 2.0f * ml - bl + tr + 2.0f * mr + br;

    // Sobel Y kernel:  [ 1  2  1]
    //                   [ 0  0  0]
    //                   [-1 -2 -1]
    float Gy = tl + 2.0f * tc + tr - bl - 2.0f * bc - br;

    // Gradient magnitude
    float edgeMagnitude = sqrt(Gx * Gx + Gy * Gy);

    // Threshold with smoothstep for crisp but anti-aliased edges
    // edge = 1.0 where edges exist, 0.0 where smooth
    // EdgeThreshold controls the upper bound (adjustable via Ctrl+Scroll)
    float upperThreshold = EdgeThreshold > 0.0f ? EdgeThreshold : 0.3f;
    float edge = smoothstep(0.05f, upperThreshold, edgeMagnitude);

    // Output edge strength (white = edge, black = no edge)
    FragColor = vec4(vec3(edge), 1.0f);
}
)"
