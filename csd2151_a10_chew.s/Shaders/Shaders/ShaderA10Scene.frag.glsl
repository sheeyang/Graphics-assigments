R"(
/*!*****************************************************************************
\file ShaderA10Scene.frag.glsl
\author GitHub Copilot
\par Course: CSD2151
\par Assignment: 10 (Portal Projection)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Scene fragment shader for Assignment 10. Renders lit geometry, projects the
    portal texture onto the scene, and renders the portal quad by blending the
    dynamic portal texture with the Pisa skybox.
*******************************************************************************/
#version 420

struct Light
{
    vec3 position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
};

struct Material
{
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    vec3 color;
    float shininess;
    float effect;
};

struct Fog
{
    float minDist;
    float maxDist;
    vec4 color;
};

struct Camera
{
    vec3 position;
};

in vec3 Position;
in vec3 Normal;
in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;
in vec4 ProjTexCoord;

uniform Light light[1];
uniform Material material;
uniform Fog fog;
uniform Camera camera;
uniform mat4 V;
uniform int PortalActive = 0;
uniform vec3 PortalPosition = vec3(0.0f);

layout(binding=0) uniform samplerCube CubeMapTex;
layout(binding=1) uniform sampler2D PortalTex;

layout(location=0) out vec4 FragColor;

bool insideProjection(vec4 projectedCoord)
{
    return projectedCoord.w > 0.0f &&
        projectedCoord.x > 0.0f && projectedCoord.x < projectedCoord.w &&
        projectedCoord.y > 0.0f && projectedCoord.y < projectedCoord.w &&
        projectedCoord.z > 0.0f && projectedCoord.z < projectedCoord.w;
}

vec3 blinnPhong(vec3 position, vec3 normal, Light lightSource, Material surface, mat4 view)
{
    vec3 lightPosInView = (view * vec4(lightSource.position, 1.0f)).xyz;
    vec3 L = normalize(lightPosInView - position);
    vec3 Vdir = normalize(-position);
    vec3 N = normalize(normal);
    vec3 H = normalize(L + Vdir);

    vec3 ambient = lightSource.La * surface.Ka;
    float NdotL = max(dot(N, L), 0.0f);
    vec3 diffuse = lightSource.Ld * surface.Kd * NdotL;

    vec3 specular = vec3(0.0f);
    if (NdotL > 0.0f)
    {
        float NdotH = max(dot(N, H), 0.0f);
        specular = lightSource.Ls * surface.Ks * pow(NdotH, surface.shininess);
    }

    return ambient + diffuse + specular;
}

vec3 applyFog(vec3 color, vec3 position)
{
    float dist = length(position);
    float fogFactor = clamp((fog.maxDist - dist) / max(fog.maxDist - fog.minDist, 0.001f), 0.0f, 1.0f);
    return mix(fog.color.rgb, color, fogFactor);
}

void main()
{
    int effect = int(material.effect + 0.5f);

    if (effect == 1)
    {
        vec4 portalSample = texture(PortalTex, TexCoord);
        if (portalSample.a < 0.05f)
            discard;

        vec3 portalNormal = normalize(WorldNormal);
        vec3 viewDir = normalize(camera.position - WorldPosition);
        vec3 reflectedDir = reflect(-viewDir, portalNormal);
        vec3 skyColor = texture(CubeMapTex, reflectedDir).rgb;

        float fresnel = pow(1.0f - max(dot(viewDir, portalNormal), 0.0f), 3.0f);
        vec3 portalColor = mix(skyColor, portalSample.rgb, portalSample.a);
        portalColor += vec3(0.18f, 0.34f, 0.85f) * fresnel;

        FragColor = vec4(portalColor, 1.0f);
        return;
    }

    vec3 litColor = blinnPhong(Position, normalize(Normal), light[0], material, V) * material.color;

    if (PortalActive == 1 && insideProjection(ProjTexCoord))
    {
        vec4 projectedPortal = textureProj(PortalTex, ProjTexCoord);
        if (projectedPortal.a > 0.001f)
        {
            vec3 projectedSky = texture(CubeMapTex, normalize(WorldPosition - PortalPosition)).rgb;
            vec3 portalBlend = mix(projectedSky, projectedPortal.rgb, 0.85f);
            litColor = mix(litColor, portalBlend, projectedPortal.a * 0.75f);
        }
    }

    FragColor = vec4(applyFog(litColor, Position), 1.0f);
}
)"