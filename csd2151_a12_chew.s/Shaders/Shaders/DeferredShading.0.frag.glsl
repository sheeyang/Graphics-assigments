R"(
    #version 420

    // This fragment shader implements the first (geometry) pass for deferred 
    // shading that stores G-buffer data: position and normalized normal vector
    // (perturbed by the normal map), and texture coordinates.

    in vec3 Position;
    in vec3 Normal;
    in vec2 TexCoord;
    in vec3 Tangent;
    in vec3 Binormal;

    layout(location=1) out vec3 gPosition;
    layout(location=2) out vec3 gNormal;
    layout(location=3) out vec2 gTexCoord;

    // Normal map bound at texture unit 5 by the window manager
    layout(binding=5) uniform sampler2D NormalMapTex;

    void main() 
    {
        // Sample normal map and decode from [0,1] to [-1,1] (tangent space)
        vec3 mapNormal = normalize(texture(NormalMapTex, TexCoord).xyz * 2.0f - 1.0f);

        // Build TBN matrix (tangent space -> view space)
        mat3 TBN = mat3(normalize(Tangent), normalize(Binormal), normalize(Normal));

        // Transform normal from tangent space to view space
        gPosition = Position;
        gNormal   = normalize(TBN * mapNormal);
        gTexCoord = TexCoord;
    }
)"
