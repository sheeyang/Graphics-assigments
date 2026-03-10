R"(
/*!*****************************************************************************
\file ShaderA10Scene.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 10 (Portal with Skybox)
\date 03/10/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 10 - Screen-space portal overlay.
    Projects portal direction to screen space, draws a fixed-size portal
    circle, and samples the procedural portal FBO texture into it.
*******************************************************************************/
#version 420

uniform vec2 iResolution;
uniform vec3 PortalDir;  // normalized direction from camera to portal
uniform mat4 SkyV;
uniform mat4 SkyP;
uniform float PortalRadius; // screen-space radius in NDC (0..1 range)

layout(binding = 1) uniform sampler2D PortalTexture;

layout(location = 0) out vec4 FragColor;

void main()
{
    // Project the portal direction as a point far along that ray
    // Using view matrix: the dir in view space gives us clip coords
    vec3 viewDir = mat3(SkyV) * PortalDir;

    // If portal is behind camera, discard
    if (viewDir.z > 0.0)
        discard;

    // Project to clip space using a point along the direction
    vec4 clipPos = SkyP * vec4(viewDir * 100.0, 1.0);
    vec2 ndcPos = clipPos.xy / clipPos.w;
    vec2 screenCenter = ndcPos * 0.5 + 0.5;

    // Current fragment in screen UV
    vec2 uv = gl_FragCoord.xy / iResolution;

    // Distance from fragment to portal center, aspect-corrected
    vec2 diff = uv - screenCenter;
    diff.x *= iResolution.x / iResolution.y;

    float dist = length(diff);

    float radius = PortalRadius;
    if (dist >= radius)
        discard;

    // Map to portal texture UV
    vec2 portalUV = (diff / radius) * 0.5 + 0.5;
    vec4 portalColor = texture(PortalTexture, portalUV);

    // Smooth edge fade
    float fade = smoothstep(radius, radius * 0.75, dist);

    FragColor = vec4(portalColor.rgb, portalColor.a * fade);
}
)"
