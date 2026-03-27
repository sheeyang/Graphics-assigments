R"(
    #version 330 core

    // Calculates the light and view direction vectors and passes them,
    // along with texture coordinates, to the fragment shader.
    // Used for normal mapping with tangent-space lighting calculation.

    layout(location=0) in vec3 VertexPosition;
    layout(location=1) in vec3 VertexNormal;
    layout(location=2) in vec2 VertexTexCoord;
    layout(location=3) in vec4 VertexTangent;

    struct Light 
    {
        vec3 position;      // Position of the light source in the world space
        vec3 La;            // Ambient light intensity
        vec3 Ld;            // Diffuse light intensity
        vec3 Ls;            // Specular light intensity
    };

    uniform Light light[1];
    uniform mat4 M;     // Model transform matrix
    uniform mat4 V;     // View transform matrix
    uniform mat4 P;     // Projection transform matrix

    out vec3 LightDirection;
    out vec3 ViewDirection;
    out vec2 TextureCoordinates;

    void main()
    {
        mat4 MV = V * M; // Model-View transform matrix
        mat3 N = mat3(transpose(inverse(MV))); // Normal transform matrix

        // Transform normal and tangent to view space and compute the binormal
        vec3 norm = normalize(N * VertexNormal);
        vec3 tang = normalize(N * vec3(VertexTangent));
        vec3 binormal = normalize(cross(norm, tang)) * VertexTangent.w;

        // Matrix for transformation to the tangent space
        mat3 toTangentSpace = mat3( tang.x, binormal.x, norm.x,
                                    tang.y, binormal.y, norm.y,
                                    tang.z, binormal.z, norm.z  );

        // Transform the light and view directions to the tangent space
        vec3 vertexPositionInView = vec3(MV * vec4(VertexPosition, 1.0f));
        vec3 lightPositionInView = vec3(V * vec4(light[0].position, 1.0f));
        LightDirection = toTangentSpace * (lightPositionInView - vertexPositionInView);
        ViewDirection = toTangentSpace * normalize(-vertexPositionInView);

        TextureCoordinates = VertexTexCoord;
        gl_Position = P * MV * vec4(VertexPosition, 1.0f);
    }
)"
