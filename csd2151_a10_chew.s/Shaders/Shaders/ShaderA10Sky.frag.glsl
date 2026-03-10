R"(
/*!*****************************************************************************
\file ShaderA10Sky.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 10 - HDR skybox with projected portal blend.
    Samples an HDR cubemap texture, applies Reinhard tone mapping and gamma
    correction, then blends a projected portal texture using projective texture
    mapping when PortalEnabled is set.
*******************************************************************************/
#version 420

in vec3 Vec;               // Direction vector from vertex shader
in vec4 ProjTexCoord;      // Projected texture coordinates for portal

uniform samplerCube CubeMapTex;                // HDR cubemap texture (Pisa)
layout(binding=1) uniform sampler2D PortalTex; // Portal texture from FBO

uniform int PortalEnabled;  // Whether portal is placed (0=no, 1=yes)

layout(location=0) out vec4 FragColor;

void main()
{
    // Sample the HDR cubemap using the direction vector
    vec3 hdrColor = texture(CubeMapTex, Vec).rgb;

    // Reinhard tone mapping: maps HDR values to [0, 1] range
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

    // Gamma correction (linear to sRGB)
    vec3 skyColor = pow(mapped, vec3(1.0 / 2.2));

    vec4 finalColor = vec4(skyColor, 1.0);

    // Blend portal texture if enabled and within projection bounds
    if (PortalEnabled == 1 &&
        ProjTexCoord.w > 0.0 &&
        ProjTexCoord.x > 0.0 && ProjTexCoord.x < ProjTexCoord.w &&
        ProjTexCoord.y > 0.0 && ProjTexCoord.y < ProjTexCoord.w)
    {
        vec4 portalColor = textureProj(PortalTex, ProjTexCoord);

        // Blend portal over skybox using portal alpha
        finalColor = mix(finalColor, vec4(portalColor.rgb, 1.0), portalColor.a * 0.8);
    }

    FragColor = finalColor;
}
)"
