R"(
/*!*****************************************************************************
\file ShaderA10Scene.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 10 - Scene rendering with projected portal
    texture blended with the skybox cubemap.

    Implements:
    - Blinn-Phong lighting for scene objects
    - Projected texture mapping for the portal effect
    - Portal texture blended with skybox cubemap using projected texture technique
    - The portal appears as a window into another dimension on the scene surfaces
*******************************************************************************/
#version 420

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
    float shininess;    // Specular shininess factor
};

in vec3 Position;       // In view space
in vec3 Normal;         // In view space
in vec2 TexCoord;
in vec3 WorldPos;       // In world space
in vec4 ProjTexCoord;

uniform Light light[1];
uniform Material material;
uniform mat4 V;         // View transform matrix

layout(binding=0) uniform samplerCube CubeMapTex;  // Skybox cubemap (Pisa HDR)
layout(binding=1) uniform sampler2D PortalTex;      // Portal texture (rendered in pass 1)

layout(location=0) out vec4 FragColor;

// Blinn-Phong lighting
vec3 blinnPhong(vec3 position, vec3 normal, Light light, Material material, mat4 view)
{
    vec3 lightPosInView = (view * vec4(light.position, 1.0f)).xyz;
    vec3 L = normalize(lightPosInView - position);
    vec3 Vdir = normalize(-position);
    vec3 N = normalize(normal);
    vec3 H = normalize(L + Vdir);

    // Ambient
    vec3 ambient = light.La * material.Ka;
    if (length(ambient) < 0.001f)
        ambient = 0.15f * material.Kd;

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

void main()
{
    vec3 shadeColor = blinnPhong(Position, normalize(Normal), light[0], material, V);

    // Check if this fragment is within the portal projection
    if (ProjTexCoord.w > 0.0f &&
            ProjTexCoord.x > 0.0f && ProjTexCoord.x < ProjTexCoord.w &&
                ProjTexCoord.y > 0.0f && ProjTexCoord.y < ProjTexCoord.w)
    {
        // Sample the portal texture using projective texturing
        vec4 portalColor = textureProj(PortalTex, ProjTexCoord);

        // Sample skybox cubemap using world position direction for environment blend
        vec3 skyDir = normalize(WorldPos);
        vec3 hdrSky = texture(CubeMapTex, skyDir).rgb;
        // Tone map the HDR skybox
        vec3 mappedSky = hdrSky / (hdrSky + vec3(1.0));
        mappedSky = pow(mappedSky, vec3(1.0 / 2.2));

        // Blend portal with skybox using portal alpha
        vec3 portalBlend = mix(mappedSky, portalColor.rgb, portalColor.a);

        // Final blend: mix shading with the portal+skybox blend
        FragColor = mix(vec4(shadeColor, 1.0f), vec4(portalBlend, 1.0f), 0.8f);
    }
    else
    {
        FragColor = vec4(shadeColor, 1.0f);
    }
}
)"
