R"(
/*!*****************************************************************************
\file ShaderA9CelCartoon.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cel-Shading Pipeline)
\date 03/03/2026 (MM/DD/YYYY)
\brief
    Pass 5 - Cartoon/cel-shading pass.
    Re-renders the scene using a simplified cartoon shading model that
    quantizes lighting values to create the characteristic flat,
    cell-shaded appearance. All objects are rendered with quantized
    diffuse and specular bands regardless of their original material effect.
*******************************************************************************/
#version 330 core

// ============================================
// Structures
// ============================================

struct Light
{
    vec3 position;
    vec3 L;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
};

struct Material
{
    float rough;
    float metal;
    vec3 color;
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float shininess;
    float effect;
};

struct Fog
{
    float minDist;
    float maxDist;
    vec4 color;
};

// ============================================
// Inputs / Uniforms
// ============================================

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform Light light[1];
uniform Material material;
uniform Fog fog;
uniform mat4 V;

layout(location=0) out vec4 FragColor;

// ============================================
// Cartoon Quantization
// ============================================

// Quantize a value into discrete bands for toon-style shading
float cartoon(float value)
{
    int levels = 3;
    float scaleFactor = 1.0f / float(levels);
    return floor(value * float(levels)) * scaleFactor;
}

// ============================================
// Blinn-Phong with Cartoon Quantization
// ============================================

vec3 blinnPhongCartoon(vec3 position, vec3 normal)
{
    vec3 lightPosInView = (V * vec4(light[0].position, 1.0f)).xyz;
    vec3 L = normalize(lightPosInView - position);
    vec3 Vdir = normalize(-position);
    vec3 N = normalize(normal);
    vec3 H = normalize(L + Vdir);

    // Use material Kd for base color; fallback if Ka is zero
    vec3 ambient = light[0].La * material.Ka;
    if (length(ambient) < 0.001f)
        ambient = 0.15f * material.Kd;

    // If Kd is zero, use material.color as a fallback
    vec3 diffuseColor = material.Kd;
    if (length(diffuseColor) < 0.001f)
        diffuseColor = material.color;

    // Diffuse with cartoon quantization
    float cosAngle = max(dot(L, N), 0.0f);
    vec3 diffuse = light[0].Ld * diffuseColor * cartoon(cosAngle);

    // Specular with cartoon quantization
    vec3 specular = vec3(0.0f);
    if (cosAngle > 0.0f)
    {
        float cosAngle2 = max(dot(H, N), 0.0f);
        float shin = material.shininess > 0.0f ? material.shininess : 32.0f;
        specular = light[0].Ls * material.Ks * cartoon(pow(cosAngle2, shin));
    }

    return ambient + diffuse + specular;
}

// ============================================
// Checkerboard Texture
// ============================================

vec3 checkerboardTexture(vec2 uv, float size)
{
    uv *= size;
    float cellX = floor(uv.x);
    float cellY = floor(uv.y);
    float checker = mod(cellX + cellY, 2.0f);
    return vec3(checker);
}

// ============================================
// Main
// ============================================

void main()
{
    vec3 n = normalize(Normal);

    // Handle discard effect objects
    int effectId = int(material.effect + 0.5f);
    if (effectId == 1)
    {
        float freq      = 12.0f;
        float amplitude = 0.25f;
        float threshold = 0.35f;
        float wave = fract(TexCoord.y * freq + sin(TexCoord.x * freq) * amplitude);
        if (wave < threshold)
            discard;
        if (!gl_FrontFacing)
            n = -n;
    }

    // Apply cartoon shading to all objects
    vec3 color = blinnPhongCartoon(Position, n);

    // For checkerboard objects, blend with checker pattern
    if (effectId == 3)
    {
        vec3 checker = checkerboardTexture(TexCoord, 10.0f);
        color = color * mix(vec3(0.3f), vec3(1.0f), checker);
    }

    // Gamma correction
    color = pow(color, vec3(1.0f / 2.2f));

    // Distance fog
    float fogDist = length(Position);
    float fogFactor = clamp((fogDist - fog.minDist) / (fog.maxDist - fog.minDist), 0.0f, 1.0f);
    FragColor = mix(vec4(color, 1.0f), fog.color, fogFactor);
}
)"
