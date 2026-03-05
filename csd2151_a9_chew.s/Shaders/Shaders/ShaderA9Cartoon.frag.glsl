R"(
/*!*****************************************************************************
\file ShaderA9Cartoon.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cartoon)
\date 03/05/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 9 - Pass 5: Cartoon/Cel Shading.
    Implements a simplified cartoon shading model that quantizes lighting
    values into discrete bands to create the characteristic flat,
    cell-shaded appearance of traditional 2D animation.
*******************************************************************************/
#version 420

// ============================================
// Structures
// ============================================

struct Light
{
    vec3 position;      // Position of the light source in world space
    vec3 La;            // Ambient light intensity
    vec3 Ld;            // Diffuse light intensity
    vec3 Ls;            // Specular light intensity
};

struct Material
{
    vec3 Ka;            // Ambient reflectivity
    vec3 Kd;            // Diffuse reflectivity
    vec3 Ks;            // Specular reflectivity
    float shininess;    // Specular shininess exponent
};

// ============================================
// Inputs / Uniforms
// ============================================

in vec3 Position;       // Fragment position in view space
in vec3 Normal;         // Fragment normal in view space

uniform Light light[1];
uniform Material material;
uniform mat4 V;         // View transform matrix

// Mode toggle: 1 = cartoon shading, 0 = standard Blinn-Phong
uniform int CartoonEnabled = 1;

layout(location=0) out vec4 FragColor;

// ============================================
// Standard Blinn-Phong (non-cartoon)
// ============================================

vec3 blinnPhong(vec3 position, vec3 normal)
{
    vec3 lightPosInView = (V * vec4(light[0].position, 1.0f)).xyz;
    vec3 L = normalize(lightPosInView - position);
    vec3 Vdir = normalize(-position);
    vec3 N = normalize(normal);
    vec3 H = normalize(L + Vdir);

    vec3 ambient = light[0].La * material.Ka;
    if (length(ambient) < 0.001f)
        ambient = 0.15f * material.Kd;

    float NdotL = max(dot(N, L), 0.0f);
    vec3 diffuse = light[0].Ld * material.Kd * NdotL;

    vec3 specular = vec3(0.0f);
    if (NdotL > 0.0f)
    {
        float NdotH = max(dot(N, H), 0.0f);
        specular = light[0].Ls * material.Ks * pow(NdotH, material.shininess);
    }

    return ambient + diffuse + specular;
}

// ============================================
// Cartoon Quantization
// ============================================

// Quantize a value into discrete bands for toon-style shading
// Using 3 levels creates visually distinct light/shadow bands
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
    // Transform light position to view space
    vec3 lightPosInView = (V * vec4(light[0].position, 1.0f)).xyz;

    // Direction vectors
    vec3 L = normalize(lightPosInView - position);
    vec3 Vdir = normalize(-position);
    vec3 N = normalize(normal);
    vec3 H = normalize(L + Vdir);

    // Ambient component (constant base illumination)
    vec3 ambient = light[0].La * material.Ka;
    if (length(ambient) < 0.001f)
        ambient = 0.15f * material.Kd;

    // Diffuse with cartoon quantization
    float cosAngle = max(dot(L, N), 0.0f);
    vec3 diffuse = light[0].Ld * material.Kd * cartoon(cosAngle);

    // Specular with cartoon quantization
    vec3 specular = vec3(0.0f);
    if (cosAngle > 0.0f)
    {
        float NdotH = max(dot(N, H), 0.0f);
        specular = light[0].Ls * material.Ks * cartoon(pow(NdotH, material.shininess));
    }

    return ambient + diffuse + specular;
}

void main()
{
    vec3 color;
    if (CartoonEnabled == 1)
        color = blinnPhongCartoon(Position, Normal);
    else
        color = blinnPhong(Position, Normal);
    FragColor = vec4(color, 1.0f);
}
)"
