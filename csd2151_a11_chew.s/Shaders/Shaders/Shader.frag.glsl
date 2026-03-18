R"(
#version 330 core

uniform vec2 iResolution;   // Viewport resolution (in pixels)

layout(location=0) out vec4 FragColor;

vec4 checkerboardTexture(vec2 uv, int size)
{
    uv = uv * size;
    float color = mod(floor(uv.x) + floor(uv.y), 2.0f) / 5.0f; // x+y even:black  odd:white
    return vec4(color, color, color, 1.0f);
}

void main()
{
    FragColor = checkerboardTexture(gl_FragCoord.xy / iResolution.xy, 10);
}
)"