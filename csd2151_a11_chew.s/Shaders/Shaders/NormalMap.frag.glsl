R"(
    #version 330 core

    // This fragment shader implements Blinn-Phong shading with normal mapping.
    // It reads the perturbed normal from a normal map texture (tangent space),
    // then applies ambient + diffuse + specular lighting using the mapped normal.

    in vec3 LightDirection;
    in vec3 ViewDirection;
    in vec2 TextureCoordinates;

    layout(location=0) out vec4 FragColor;

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

    uniform Light light[1];
    uniform Material material;

    // Color texture (diffuse) and normal map bound at texture units 0 and 1
    uniform sampler2D ColorTex;     // Diffuse color texture (binding=0)
    uniform sampler2D NormalMapTex; // Normal map texture (binding=1)

    uniform int Option = 1;

    void main()
    {
        // Sample the normal map and transform from [0,1] to [-1,1]
        vec3 normal = normalize(texture(NormalMapTex, TextureCoordinates).xyz * 2.0f - 1.0f);

        // Sample the diffuse color texture
        vec4 texColor = texture(ColorTex, TextureCoordinates);

        vec3 lightDir = normalize(LightDirection);
        vec3 viewDir  = normalize(ViewDirection);

        // Ambient
        vec3 ambient = light[0].La * material.Ka * texColor.rgb;

        // Diffuse
        float lDotN = max(dot(lightDir, normal), 0.0f);
        vec3 diffuse = light[0].Ld * material.Kd * texColor.rgb * lDotN;

        // Specular (Blinn-Phong half-vector)
        vec3 spec = vec3(0.0f);
        if (lDotN > 0.0f)
        {
            vec3 halfway = normalize(viewDir + lightDir);
            spec = light[0].Ls * material.Ks *
                   pow(max(dot(halfway, normal), 0.0f), material.shininess);
        }

        vec3 color = ambient + diffuse + spec;

        // Option 1: Full shading with normal map
        // Option 2: Normal-map normals visualized
        // Option 3: Diffuse texture only
        if (Option == 2)
            FragColor = vec4(normal * 0.5f + 0.5f, 1.0f);
        else if (Option == 3)
            FragColor = vec4(texColor.rgb, 1.0f);
        else
            FragColor = vec4(pow(color, vec3(1.0f / 2.2f)), 1.0f);
    }
)"
