R"(
/*!*****************************************************************************
\file ShaderA4.frag.glsl
\author Vadim Surov (vsurov@digipen.edu)
\co-author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 4 (Blinn-Phong Shading)
\date 02/01/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 4 - Blinn-Phong Shading implementation.
    Implements per-pixel Blinn-Phong illumination model with:
    - Ambient component
    - Diffuse component (Lambert's Cosine Law)
    - Specular component (Blinn-Phong half-vector method)
    
    Features:
    - Single point light source
    - Configurable material properties
    - View-space lighting calculations
*******************************************************************************/
#version 330 core

// Light structure
struct Light 
{
    vec3 position;      // Position of the light source in world space
    vec3 La;            // Ambient light intensity
    vec3 Ld;            // Diffuse light intensity
    vec3 Ls;            // Specular light intensity
};

// Material structure
struct Material 
{
    vec3 Ka;            // Ambient reflectivity
    vec3 Kd;            // Diffuse reflectivity
    vec3 Ks;            // Specular reflectivity
    float shininess;    // Specular shininess exponent
};

// Inputs from vertex shader
in vec3 Position;       // Fragment position in view space
in vec3 Normal;         // Fragment normal in view space

// Uniforms
uniform Light light[1];
uniform Material material;
uniform mat4 V;         // View transform matrix (to transform light position)

// Output
layout(location=0) out vec4 FragColor;

// ============================================
// Blinn-Phong Illumination Function
// ============================================
// Computes the lighting contribution using the Blinn-Phong model
// Parameters:
//   position - fragment position in view space
//   normal   - normalized surface normal in view space
//   light    - light source properties
//   material - surface material properties
//   view     - view matrix for transforming light position
// Returns:
//   Final color with ambient + diffuse + specular components
vec3 blinnPhong(vec3 position, vec3 normal, Light light, Material material, mat4 view)
{
    // Transform light position to view space
    vec3 lightPosInView = (view * vec4(light.position, 1.0f)).xyz;
    
    // Calculate vectors needed for lighting
    // L: direction from fragment to light source
    vec3 L = normalize(lightPosInView - position);
    
    // V: direction from fragment to viewer (camera is at origin in view space)
    vec3 V = normalize(-position);
    
    // N: surface normal (already in view space, but ensure normalized)
    vec3 N = normalize(normal);
    
    // H: half-vector between light direction and view direction (Blinn-Phong)
    vec3 H = normalize(L + V);
    
    // ========================================
    // Ambient Component
    // ========================================
    // Ambient light is independent of geometry - provides base illumination
    vec3 ambient = light.La * material.Ka;
    
    // ========================================
    // Diffuse Component (Lambert's Cosine Law)
    // ========================================
    // Diffuse reflection depends on angle between normal and light direction
    // max() prevents negative values when light is behind the surface
    float NdotL = max(dot(N, L), 0.0f);
    vec3 diffuse = light.Ld * material.Kd * NdotL;
    
    // ========================================
    // Specular Component (Blinn-Phong)
    // ========================================
    // Specular highlight uses half-vector H instead of reflection vector R
    // This is computationally cheaper and produces similar results
    // The shininess exponent controls highlight size (higher = smaller, sharper)
    vec3 specular = vec3(0.0f);
    if (NdotL > 0.0f)
    {
        float NdotH = max(dot(N, H), 0.0f);
        specular = light.Ls * material.Ks * pow(NdotH, material.shininess);
    }
    
    // ========================================
    // Combine Components
    // ========================================
    // Final color = ambient + diffuse + specular
    return ambient + diffuse + specular;
}

void main() 
{
    // Compute per-pixel Blinn-Phong shading
    vec3 color = blinnPhong(Position, normalize(Normal), light[0], material, V);
    
    // Output final fragment color with full opacity
    FragColor = vec4(color, 1.0f);
}
)"
