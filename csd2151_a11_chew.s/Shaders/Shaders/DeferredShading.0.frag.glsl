R"(
    #version 330 core

    // This fragment shader implements the first (geometry) pass for deferred 
    // shading that stores G-buffer data: position and normalized normal vector,
    // and texture coordinates.

    in vec3 Position;
    in vec3 Normal;
    in vec2 TexCoord;

    layout(location=1) out vec3 gPosition;
    layout(location=2) out vec3 gNormal;
    layout(location=3) out vec2 gTexCoord;

    void main() 
    {
        gPosition = Position;
        gNormal = normalize(Normal);
        gTexCoord = TexCoord;
    }
)"
