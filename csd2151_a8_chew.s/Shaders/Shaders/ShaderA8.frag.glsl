R"(
/*!*****************************************************************************
\file ShaderA8.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 8 (Unified Rendering)
\date 02/23/2026 (MM/DD/YYYY)
\brief
    Unified fragment shader for Assignment 8 - scene rendering.
    Implements multiple rendering effects selected by material.effect:
      0.0 = Physically Based Rendering (PBR) using GGX microfacet model
      1.0 = Discard effect with circular holes (custom pattern)
      2.0 = Cartoon/toon shading with quantized lighting bands
      3.0 = Checkerboard textured surface with Blinn-Phong lighting
    All effects include distance-based fog for atmospheric depth.
*******************************************************************************/
#version 330 core

// ============================================
// Structures
// ============================================

struct Light
{
    vec3 position;      // Position of the light source in world space

    vec3 L;             // Light intensity (for PBR)

    vec3 La;            // Ambient light intensity (for Phong)
    vec3 Ld;            // Diffuse light intensity (for Phong)
    vec3 Ls;            // Specular light intensity (for Phong)
};

struct Material
{
    // PBR properties
    float rough;        // Roughness
    float metal;        // Metallic (1.0) or dielectric (0.0)
    vec3 color;         // Diffuse color for metal / base color

    // Phong properties
    vec3 Ka;            // Ambient reflectivity
    vec3 Kd;            // Diffuse reflectivity
    vec3 Ks;            // Specular reflectivity
    float shininess;    // Specular shininess factor

    // Effect selector
    float effect;       // 0=PBR, 1=Discard, 2=Cartoon, 3=Checkerboard
};

struct Fog
{
    float minDist;      // Distance where fog starts
    float maxDist;      // Distance where fog is fully opaque
    vec4 color;         // Fog color
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
uniform mat4 V;         // View transform matrix

layout(location=0) out vec4 FragColor;

// ============================================
// Constants
// ============================================

const float PI = 3.14159265358979323846f;

// ============================================
// PBR Functions (GGX Microfacet Model)
// ============================================

// GGX/Trowbridge-Reitz Normal Distribution Function
// Describes the concentration of microfacet normals in direction H
float ggxDistribution(float nDotH)
{
    float alpha2 = material.rough * material.rough * material.rough * material.rough;
    float d = (nDotH * nDotH) * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / (PI * d * d);
}

// Smith Masking-Shadowing Function
// Probability that microfacets are visible from both light and view directions
float geomSmith(float nDotL)
{
    float k = (material.rough + 1.0f) * (material.rough + 1.0f) / 8.0f;
    float denom = nDotL * (1.0f - k) + k;
    return 1.0f / denom;
}

// Schlick Fresnel approximation
// Probability of light reflected from an optically flat surface
vec3 schlickFresnel(float lDotH)
{
    vec3 f0 = vec3(0.04f); // Dielectrics
    if (material.metal == 1.0f)
        f0 = material.color;
    return f0 + (1.0f - f0) * pow(1.0f - lDotH, 5);
}

// Microfacet BRDF model combining distribution, fresnel, and geometry terms
vec3 microfacetModel(vec3 position, vec3 n)
{
    vec3 diffuseBrdf = material.color;

    vec3 lightI = light[0].L;
    vec3 lightPositionInView = (V * vec4(light[0].position, 1.0f)).xyz;

    vec3 l = lightPositionInView - position;
    float dist = length(l);
    l = normalize(l);
    lightI *= 100.0f / (dist * dist); // Intensity normalization with distance

    vec3 v = normalize(-position);
    vec3 h = normalize(v + l);
    float nDotH = dot(n, h);
    float lDotH = dot(l, h);
    float nDotL = max(dot(n, l), 0.0f);
    float nDotV = dot(n, v);
    vec3 specBrdf = 0.25f * ggxDistribution(nDotH) * schlickFresnel(lDotH) *
                      geomSmith(nDotL) * geomSmith(nDotV);

    return (diffuseBrdf + PI * specBrdf) * lightI * nDotL;
}

// ============================================
// Blinn-Phong Lighting
// ============================================

vec3 blinnPhong(vec3 position, vec3 normal, Light light, Material material, mat4 view)
{
    vec3 lightPosInView = (view * vec4(light.position, 1.0f)).xyz;
    vec3 L = normalize(lightPosInView - position);
    vec3 Vdir = normalize(-position);
    vec3 N = normalize(normal);
    vec3 H = normalize(L + Vdir);

    // Ambient
    vec3 ambient = light.La * material.Ka;

    // Diffuse
    float NdotL = max(dot(N, L), 0.0f);
    vec3 diffuse = light.Ld * material.Kd * NdotL;

    // Specular
    vec3 specular = vec3(0.0f);
    if (NdotL > 0.0f)
    {
        float NdotH = max(dot(N, H), 0.0f);
        specular = light.Ls * material.Ks * pow(NdotH, material.shininess);
    }

    return ambient + diffuse + specular;
}

// ============================================
// Cartoon Shading
// ============================================

// Quantize a value into discrete bands for toon-style shading
float cartoon(float value)
{
    int levels = 2;
    float scaleFactor = 1.0f / float(levels);
    return floor(value * float(levels)) * scaleFactor;
}

// Blinn-Phong with cartoon quantization applied to diffuse and specular
vec3 blinnPhongCartoon(vec3 position, vec3 normal, Light light, Material material, mat4 view)
{
    vec3 lightPosInView = (view * vec4(light.position, 1.0f)).xyz;
    vec3 L = normalize(lightPosInView - position);
    vec3 Vdir = normalize(-position);
    vec3 N = normalize(normal);
    vec3 H = normalize(L + Vdir);

    // Ambient
    vec3 ambient = light.La * material.Ka;

    // Diffuse with cartoon quantization
    float cosAngle = max(dot(L, N), 0.0f);
    vec3 diffuse = light.Ld * material.Kd * cartoon(cosAngle);

    // Specular with cartoon quantization
    vec3 specular = vec3(0.0f);
    if (cosAngle > 0.0f)
    {
        float cosAngle2 = max(dot(H, N), 0.0f);
        specular = light.Ls * material.Ks * cartoon(pow(cosAngle2, material.shininess));
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
// Main Function
// ============================================

void main()
{
    vec3 n = normalize(Normal);
    vec3 color;
    int effectId = int(material.effect + 0.5f);

    // ---- Effect 0: PBR (Physically Based Rendering) ----
    if (effectId == 0)
    {
        vec3 shade = microfacetModel(Position, n);

        // Subtle checkerboard texture blend
        vec3 tex = checkerboardTexture(TexCoord, 10.0f);
        color = mix(shade, tex, 0.1f);

        // Gamma correction
        color = pow(color, vec3(1.0f / 2.2f));
    }
    // ---- Effect 1: Discard with circular holes (custom pattern) ----
    else if (effectId == 1)
    {
        // Create a grid of circular holes
        vec2 cell = fract(TexCoord * 8.0f);
        float dist = distance(cell, vec2(0.5f));

        // Discard fragments inside circles (creates round holes)
        if (dist < 0.3f)
            discard;

        // Render surviving fragments with Blinn-Phong
        // Invert normal if back-facing for correct lighting through holes
        if (!gl_FrontFacing)
            n = -n;

        color = blinnPhong(Position, n, light[0], material, V);
    }
    // ---- Effect 2: Cartoon / Toon Shading ----
    else if (effectId == 2)
    {
        color = blinnPhongCartoon(Position, n, light[0], material, V);
    }
    // ---- Effect 3: Checkerboard plane with lighting ----
    else
    {
        vec3 phong = blinnPhong(Position, n, light[0], material, V);
        vec3 checker = checkerboardTexture(TexCoord, 10.0f);

        // Blend lighting with checkerboard pattern
        color = phong * mix(vec3(0.3f), vec3(1.0f), checker);
    }

    // ---- Apply Fog ----
    float fogDist = length(Position);
    float fogFactor = clamp((fogDist - fog.minDist) / (fog.maxDist - fog.minDist), 0.0f, 1.0f);
    FragColor = mix(vec4(color, 1.0f), fog.color, fogFactor);
}
)"
