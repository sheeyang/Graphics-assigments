R"(
    #version 330 core

    // Pass-through vertex shader for fullscreen quad rendering.
    // Used for the second (lighting) pass of deferred shading.

    layout(location=0) in vec3 VertexPosition;

    void main()
    {
        gl_Position = vec4(VertexPosition.x, VertexPosition.y, 0.0f, 1.0f);
    }
)"
