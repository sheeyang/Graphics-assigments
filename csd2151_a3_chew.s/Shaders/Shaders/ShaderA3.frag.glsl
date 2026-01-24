R"(
/*!*****************************************************************************
\file ShaderA3.frag.glsl
\author Chew Shee Yang (chew.s@digipen.edu)
\par Course: CSD2151
\par Assignment: 3 (Ray Tracing)
\date 01/24/2026 (MM/DD/YYYY)
\brief
    Fragment shader for Assignment 3 - Ray Tracing implementation.
    Implements a simple ray tracer with:
    - Earth: Stationary sphere at center
    - Sun: Orbiting light source around Earth
    - Moon: Orbiting sphere around Earth
    - Lambert shading (ambient + diffuse) based on Sun position
*******************************************************************************/
#version 330 core

uniform vec2 iResolution;   // Viewport resolution (in pixels)
uniform float iTime;        // Time in seconds

layout(location=0) out vec4 FragColor;

// Constants
const float PI = 3.14159265359;

// Camera settings
const vec3 CAMERA_POS = vec3(0.0, 0.0, 5.0);  // Camera position (eye)
const float FOV = 60.0;                        // Field of view in degrees

// Object parameters
const vec3 EARTH_CENTER = vec3(0.0, 0.0, 0.0);
const float EARTH_RADIUS = 1.0;
const vec3 EARTH_COLOR = vec3(0.2, 0.5, 0.8);  // Blue-ish Earth

const float MOON_RADIUS = 0.25;
const float MOON_ORBIT_RADIUS = 2.0;
const float MOON_ORBIT_SPEED = 0.5;            // Radians per second
const vec3 MOON_COLOR = vec3(0.7, 0.7, 0.7);   // Gray Moon

const float SUN_RADIUS = 0.4;
const float SUN_ORBIT_RADIUS = 3.5;
const float SUN_ORBIT_SPEED = 0.3;             // Radians per second
const vec3 SUN_COLOR = vec3(1.0, 0.9, 0.5);    // Yellow Sun
const vec3 SUN_LIGHT_INTENSITY = vec3(1.0, 0.95, 0.8);

// Ambient light
const vec3 AMBIENT_LIGHT = vec3(0.1, 0.1, 0.1);

// ============================================
// Ray-Sphere Intersection
// ============================================
// Returns t (distance along ray) or -1.0 if no intersection
// Ray: P = O + t*D
// Sphere: |P - S|^2 = r^2
// Solving: (O + tD - S) · (O + tD - S) = r^2
// Quadratic: At^2 + Bt + C = 0 where A=D·D=1 (normalized), B=2(O-S)·D, C=(O-S)·(O-S)-r^2

float intersectSphere(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float sphereRadius)
{
    vec3 oc = rayOrigin - sphereCenter;  // O - S
    
    // A = D · D = 1 (rayDir is normalized)
    float B = 2.0 * dot(oc, rayDir);
    float C = dot(oc, oc) - sphereRadius * sphereRadius;
    
    float discriminant = B * B - 4.0 * C;  // B^2 - 4AC, with A=1
    
    if (discriminant < 0.0)
    {
        return -1.0;  // No intersection
    }
    
    float sqrtDisc = sqrt(discriminant);
    
    // Two solutions: t = (-B ± sqrt(discriminant)) / 2A, with A=1
    float t1 = (-B - sqrtDisc) / 2.0;
    float t2 = (-B + sqrtDisc) / 2.0;
    
    // Return the smallest positive t
    if (t1 > 0.001)
        return t1;
    if (t2 > 0.001)
        return t2;
    
    return -1.0;  // Both intersections behind camera
}

// ============================================
// Get Sun Position (orbiting)
// ============================================
vec3 getSunPosition()
{
    float angle = iTime * SUN_ORBIT_SPEED;
    return vec3(
        SUN_ORBIT_RADIUS * cos(angle),
        0.5 * sin(angle * 0.7),  // Slight vertical oscillation
        SUN_ORBIT_RADIUS * sin(angle)
    );
}

// ============================================
// Get Moon Position (orbiting)
// ============================================
vec3 getMoonPosition()
{
    float angle = iTime * MOON_ORBIT_SPEED + PI;  // Start on opposite side from sun
    return vec3(
        MOON_ORBIT_RADIUS * cos(angle),
        0.3 * sin(angle * 1.3),  // Slight vertical oscillation
        MOON_ORBIT_RADIUS * sin(angle)
    );
}

// ============================================
// Lambert Shading (Ambient + Diffuse)
// ============================================
vec3 lambertShading(vec3 point, vec3 normal, vec3 objectColor, vec3 lightPos)
{
    // Ambient component
    vec3 ambient = AMBIENT_LIGHT * objectColor;
    
    // Diffuse component (Lambert's cosine law)
    vec3 lightDir = normalize(lightPos - point);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * SUN_LIGHT_INTENSITY * objectColor;
    
    return ambient + diffuse;
}

// ============================================
// Ray Tracing Main Logic
// ============================================
vec3 trace(vec3 rayOrigin, vec3 rayDir)
{
    // Get current positions of orbiting objects
    vec3 sunPos = getSunPosition();
    vec3 moonPos = getMoonPosition();
    
    // Background color (space - dark with stars)
    vec3 backgroundColor = vec3(0.02, 0.02, 0.05);
    
    // Add some stars to background
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    float starField = fract(sin(dot(floor(uv * 500.0), vec2(12.9898, 78.233))) * 43758.5453);
    if (starField > 0.998)
    {
        backgroundColor += vec3(0.5, 0.5, 0.6) * (starField - 0.998) * 500.0;
    }
    
    // Track closest intersection
    float closestT = 1e10;
    vec3 finalColor = backgroundColor;
    
    // ========================================
    // Test intersection with Sun (light source)
    // ========================================
    float tSun = intersectSphere(rayOrigin, rayDir, sunPos, SUN_RADIUS);
    if (tSun > 0.0 && tSun < closestT)
    {
        closestT = tSun;
        // Sun is emissive - glows brightly
        vec3 sunPoint = rayOrigin + tSun * rayDir;
        vec3 sunNormal = normalize(sunPoint - sunPos);
        
        // Create glowing effect for sun
        float glow = 1.0 - dot(sunNormal, -rayDir);
        glow = pow(glow, 2.0);
        finalColor = SUN_COLOR * (1.0 + glow * 0.5);
    }
    
    // ========================================
    // Test intersection with Earth
    // ========================================
    float tEarth = intersectSphere(rayOrigin, rayDir, EARTH_CENTER, EARTH_RADIUS);
    if (tEarth > 0.0 && tEarth < closestT)
    {
        closestT = tEarth;
        
        // Calculate intersection point and normal
        vec3 earthPoint = rayOrigin + tEarth * rayDir;
        vec3 earthNormal = normalize(earthPoint - EARTH_CENTER);
        
        // Apply Lambert shading
        finalColor = lambertShading(earthPoint, earthNormal, EARTH_COLOR, sunPos);
        
        // Add some surface detail (simple continent-like pattern)
        float pattern = sin(earthNormal.x * 8.0) * sin(earthNormal.y * 8.0) * sin(earthNormal.z * 8.0);
        if (pattern > 0.3)
        {
            // Land areas (greenish)
            vec3 landColor = vec3(0.3, 0.6, 0.2);
            finalColor = lambertShading(earthPoint, earthNormal, landColor, sunPos);
        }
    }
    
    // ========================================
    // Test intersection with Moon
    // ========================================
    float tMoon = intersectSphere(rayOrigin, rayDir, moonPos, MOON_RADIUS);
    if (tMoon > 0.0 && tMoon < closestT)
    {
        closestT = tMoon;
        
        // Calculate intersection point and normal
        vec3 moonPoint = rayOrigin + tMoon * rayDir;
        vec3 moonNormal = normalize(moonPoint - moonPos);
        
        // Apply Lambert shading
        finalColor = lambertShading(moonPoint, moonNormal, MOON_COLOR, sunPos);
        
        // Add crater-like details (use normal for local-space pattern)
        float craters = sin(moonNormal.x * 20.0) * sin(moonNormal.y * 20.0) * sin(moonNormal.z * 20.0);
        if (craters > 0.5)
        {
            vec3 craterColor = MOON_COLOR * 0.7;
            finalColor = lambertShading(moonPoint, moonNormal, craterColor, sunPos);
        }
    }
    
    // ========================================
    // Add sun glow effect (corona)
    // ========================================
    if (closestT > 1e9)  // If we hit background
    {
        // Calculate closest approach to sun
        vec3 toSun = sunPos - rayOrigin;
        float projLen = dot(toSun, rayDir);
        if (projLen > 0.0)
        {
            vec3 closestPoint = rayOrigin + projLen * rayDir;
            float distToSun = length(closestPoint - sunPos);
            
            // Glow falloff
            float glow = SUN_RADIUS * 3.0 / (distToSun + 0.1);
            glow = pow(glow, 2.0) * 0.3;
            finalColor += SUN_COLOR * glow;
        }
    }
    
    return finalColor;
}

// ============================================
// Main Function
// ============================================
void main()
{
    // Convert fragment coordinates to normalized device coordinates [-1, 1]
    vec2 uv = (gl_FragCoord.xy - 0.5 * iResolution.xy) / iResolution.y;
    
    // Calculate ray direction based on FOV
    float fovRad = FOV * PI / 180.0;
    float z = 1.0 / tan(fovRad / 2.0);
    
    vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
    
    // Trace the ray
    vec3 color = trace(CAMERA_POS, rayDir);
    
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));
    
    // Clamp to valid range
    color = clamp(color, 0.0, 1.0);
    
    FragColor = vec4(color, 1.0);
}
)"
