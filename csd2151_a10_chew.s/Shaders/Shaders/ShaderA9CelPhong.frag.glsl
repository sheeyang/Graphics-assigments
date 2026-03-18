R"(
/*!*****************************************************************************
\file ShaderA9CelPhong.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cel-Shading Pipeline)
\date 03/03/2026 (MM/DD/YYYY)
\brief
    Pass 1 - Blinn-Phong shading pass for cel-shading pipeline.
    Renders the scene using standard Blinn-Phong shading to capture accurate
    geometric and lighting information. The output is used as input for edge
    detection in subsequent passes.
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

in vec3 Position;       // Fragment position in view space
in vec3 Normal;         // Fragment normal in view space
in vec2 TexCoord;       // Texture coordinates

uniform Light light[1];
uniform Material material;
uniform Fog fog;
uniform mat4 V;

layout(location=0) out vec4 FragColor;

// ============================================
// Blinn-Phong Lighting (standard, no quantization)
// ============================================

vec3 blinnPhong(vec3 position, vec3 normal)
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

    float NdotL = max(dot(N, L), 0.0f);
    vec3 diffuse = light[0].Ld * diffuseColor * NdotL;

    vec3 specular = vec3(0.0f);
    if (NdotL > 0.0f)
    {
        float NdotH = max(dot(N, H), 0.0f);
        float shin = material.shininess > 0.0f ? material.shininess : 32.0f;
        specular = light[0].Ls * material.Ks * pow(NdotH, shin);
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

    // For discard effect objects, handle back faces
    int effectId = int(material.effect + 0.5f);
    if (effectId == 1)
    {
        // Sine-wave discard pattern
        float freq      = 12.0f;
        float amplitude = 0.25f;
        float threshold = 0.35f;
        float wave = fract(TexCoord.y * freq + sin(TexCoord.x * freq) * amplitude);
        if (wave < threshold)
            discard;
        if (!gl_FrontFacing)
            n = -n;
    }

    // Apply Blinn-Phong to all objects
    vec3 color = blinnPhong(Position, n);

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
