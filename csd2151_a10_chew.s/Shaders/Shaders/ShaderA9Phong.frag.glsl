R"(
/*!*****************************************************************************
\file ShaderA9Phong.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 9 (Cartoon)
\date 03/05/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 9 - Pass 1: Standard Blinn-Phong shading.
    Renders the scene with accurate geometric and lighting information for
    subsequent edge detection. The smooth gradients from Blinn-Phong provide
    reliable luminance data for the Sobel operator.
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

layout(location=0) out vec4 FragColor;

// ============================================
// Blinn-Phong Illumination
// ============================================

vec3 blinnPhong(vec3 position, vec3 normal)
{
    // Transform light position to view space
    vec3 lightPosInView = (V * vec4(light[0].position, 1.0f)).xyz;

    // Direction vectors
    vec3 L = normalize(lightPosInView - position);
    vec3 Vdir = normalize(-position);
    vec3 N = normalize(normal);
    vec3 H = normalize(L + Vdir);

    // Ambient component
    vec3 ambient = light[0].La * material.Ka;

    // Diffuse component (Lambert)
    float NdotL = max(dot(N, L), 0.0f);
    vec3 diffuse = light[0].Ld * material.Kd * NdotL;

    // Specular component (Blinn-Phong)
    vec3 specular = vec3(0.0f);
    if (NdotL > 0.0f)
    {
        float NdotH = max(dot(N, H), 0.0f);
        specular = light[0].Ls * material.Ks * pow(NdotH, material.shininess);
    }

    return ambient + diffuse + specular;
}

void main()
{
    vec3 color = blinnPhong(Position, Normal);
    FragColor = vec4(color, 1.0f);
}
)"
