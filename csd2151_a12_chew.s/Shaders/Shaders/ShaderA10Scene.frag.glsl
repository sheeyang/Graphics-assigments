R"(
/*!*****************************************************************************
\file ShaderA10Scene.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal with Skybox)
\date 03/28/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 10 - Screen-space portal overlay.
    Projects portal direction to screen space, draws a fixed-size portal
    circle, and samples the procedural portal FBO texture into it.
*******************************************************************************/
#version 420

uniform vec2 iResolution;
uniform vec3 PortalPos;  // portal center in world space
uniform vec3 PortalNormal; // fixed portal facing direction in world space
uniform mat4 SkyV;
uniform mat4 SkyP;
uniform float PortalRadius; // portal radius in world units

layout(binding = 1) uniform sampler2D PortalTexture;

layout(location = 0) out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / iResolution;
    vec2 ndc = uv * 2.0 - 1.0;

    mat4 invP = inverse(SkyP);
    mat4 invV = inverse(SkyV);

    vec4 clipPos = vec4(ndc, -1.0, 1.0);
    vec4 viewRay = invP * clipPos;
    viewRay = vec4(viewRay.xy, -1.0, 0.0);

    vec3 rayOrigin = vec3(invV * vec4(0.0, 0.0, 0.0, 1.0));
    vec3 rayDir = normalize(vec3(invV * viewRay));

    vec3 normal = normalize(PortalNormal);
    float denom = dot(rayDir, normal);
    if (abs(denom) < 1e-4)
        discard;

    float t = dot(PortalPos - rayOrigin, normal) / denom;
    if (t <= 0.0)
        discard;

    vec3 hitPos = rayOrigin + rayDir * t;

    // Build a stable local basis for the portal plane.
    vec3 refUp = abs(dot(normal, vec3(0.0, 1.0, 0.0))) > 0.98 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    vec3 portalRight = normalize(cross(refUp, normal));
    vec3 portalUp = normalize(cross(normal, portalRight));

    vec3 local = hitPos - PortalPos;
    vec2 planeUV = vec2(dot(local, portalRight), dot(local, portalUp));

    if (dot(planeUV, planeUV) >= PortalRadius * PortalRadius)
        discard;

    vec2 portalUV = planeUV / (PortalRadius * 2.0) + 0.5;
    vec4 portalColor = texture(PortalTexture, portalUV);

    // Smooth edge fade
    float radialDist = length(planeUV);
    float fade = 1.0 - smoothstep(PortalRadius * 0.75, PortalRadius, radialDist);

    FragColor = vec4(portalColor.rgb, portalColor.a * fade);
}
)"
