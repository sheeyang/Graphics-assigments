R"(
/*!*****************************************************************************
\file ShaderA10Scene.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal with Skybox)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 10 - Scene with projected portal texture.
    Renders objects with basic Blinn-Phong shading and blends with a
    projected portal texture using the projected texture technique.
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
    float shininess;
};

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;
in vec4 ProjTexCoord;

uniform Light light[1];
uniform Material material;
uniform mat4 V;

layout(binding=1) uniform sampler2D PortalTexture;

layout(location=0) out vec4 FragColor;

vec3 blinnPhong(vec3 pos, vec3 n, Light l, Material m, mat4 view)
{
    vec3 lpos = vec3(view * vec4(l.position, 1.0));
    vec3 s = normalize(lpos - pos);
    vec3 v = normalize(-pos);
    vec3 h = normalize(s + v);

    vec3 ambient  = l.La * m.Ka;
    vec3 diffuse  = l.Ld * m.Kd * max(dot(n, s), 0.0);
    vec3 specular = l.Ls * m.Ks * pow(max(dot(n, h), 0.0), m.shininess);

    return ambient + diffuse + specular;
}

void main()
{
    vec3 n = normalize(Normal);
    vec4 shadeColor = vec4(blinnPhong(Position, n, light[0], material, V), 1.0);

    // Projected texture blending
    if (ProjTexCoord.w > 0.0 &&
        ProjTexCoord.x > 0.0 && ProjTexCoord.x < ProjTexCoord.w &&
        ProjTexCoord.y > 0.0 && ProjTexCoord.y < ProjTexCoord.w)
    {
        vec4 portalColor = textureProj(PortalTexture, ProjTexCoord);
        FragColor = mix(shadeColor, portalColor, portalColor.a * 0.7);
    }
    else
    {
        FragColor = shadeColor;
    }
}
)"
