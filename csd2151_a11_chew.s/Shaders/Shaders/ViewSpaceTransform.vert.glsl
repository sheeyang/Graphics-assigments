R"(
    #version 330 core

    // Transforms vertex position and normal to view space and passes
    // them along with texture coordinates to the fragment shader.
    // Used for the first (geometry) pass of deferred shading.

    layout(location=0) in vec3 VertexPosition;
    layout(location=1) in vec3 VertexNormal;
    layout(location=2) in vec2 VertexTexCoord;

    uniform mat4 M;     // Model transform matrix
    uniform mat4 V;     // View transform matrix
    uniform mat4 P;     // Projection transform matrix

    out vec3 Position;  // Position in view space
    out vec3 Normal;    // Normal in view space
    out vec2 TexCoord;

    void main()
    {
        mat4 MV = V * M;
        mat3 N = mat3(transpose(inverse(MV)));

        Normal = normalize(N * VertexNormal);

        vec4 VertexPositionInView = MV * vec4(VertexPosition, 1.0f);
        Position = vec3(VertexPositionInView);

        TexCoord = VertexTexCoord;

        gl_Position = P * VertexPositionInView;
    }
)"
