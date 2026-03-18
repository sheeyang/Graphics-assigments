R"(
    #version 420

    // This fragment shader implements the second (lighting) pass for deferred 
    // shading that applies lighting calculations using the stored surface data.
    // It reads from the G-buffer (position, normal, texture coord) and applies
    // the Blinn-Phong illumination model with a color texture.

    layout(location=0) out vec4 FragColor;

    layout(binding=0) uniform sampler2D gPosition;
    layout(binding=1) uniform sampler2D gNormal;
    layout(binding=2) uniform sampler2D gTexture;

    // Window textures (diffuse and normal map bound by the window manager)
    layout(binding=4) uniform sampler2D ColorTex;

    struct Light 
    {
        vec3 position;      // Position of the light source in the world space
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

    uniform Material material;
    uniform Light light[1];
    uniform mat4 V;         // View transform matrix

    uniform int Option = 1;

    // Blinn-Phong shading function for deferred shading with 
    // additional check to handle background pixels with zero normals
    vec3 blinnPhong(vec3 position, vec3 normal, vec3 texColor, Light light, Material material, mat4 view)
    {
        vec3 color = vec3(0.0f);

        if (any(notEqual(normal, vec3(0.0f, 0.0f, 0.0f))))
        {
            vec3 ambient = light.La * material.Ka * texColor;

            // Transform light position to view space
            vec3 lightPosView = vec3(view * vec4(light.position, 1.0f));
            vec3 toLight = lightPosView - position;

            if (any(notEqual(toLight, vec3(0.0f, 0.0f, 0.0f))))
            {
                vec3 lightDir = normalize(toLight);
                vec3 viewDir = normalize(-position);

                float lDotN = max(dot(lightDir, normal), 0.0f);
                vec3 diffuse = texColor * light.Ld * material.Kd * lDotN;

                vec3 spec = vec3(0.0f);
                if (lDotN > 0.0f)
                {
                    vec3 halfway = normalize(viewDir + lightDir);
                    spec = light.Ls * material.Ks * 
                           pow(max(dot(halfway, normal), 0.0f), material.shininess);
                }

                color = ambient + diffuse + spec;
            }
            else
            {
                color = ambient;
            }
        }
        return color;
    }

    void main() 
    {
        // Retrieve position and normal information from G-buffer
        vec3 position = vec3(texelFetch(gPosition, ivec2(gl_FragCoord), 0));
        vec3 normal = vec3(texelFetch(gNormal, ivec2(gl_FragCoord), 0));
        vec2 texCoord = vec2(texelFetch(gTexture, ivec2(gl_FragCoord), 0));

        // Sample the color texture using UV from G-buffer
        vec3 texColor = vec3(texture(ColorTex, texCoord));

        if (Option == 2)
        {
            // Visualize normals: map from [-1,1] to [0,1]
            FragColor = vec4(normal * 0.5f + 0.5f, 1.0f);
        }
        else if (Option == 3)
        {
            // Diffuse texture only
            FragColor = vec4(texColor, 1.0f);
        }
        else
        {
            // Option 1 (default): Apply Blinn-Phong shading
            vec3 color = blinnPhong(position, normal, texColor, light[0], material, V);

            // Apply gamma correction
            FragColor = vec4(pow(color, vec3(1.0f/2.2f)), 1.0f);
        }
    }
)"
