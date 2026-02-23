/*!*****************************************************************************
\file CG/fbo.h
\author Vadim Surov (vsurov@digipen.edu)
\date 12/23/2025 (MM/DD/YYYY)
\brief Framebuffer Object interface: declares FBO types and API to bind,
       allocate color/HDR/depth textures, and configure deferred targets.
*******************************************************************************/
#pragma once

#include <GL/glew.h>

namespace cg
{
    //
    // This struct that sets up and manages OpenGL Framebuffer Objects (FBOs), allowing 
    // different rendering techniques(e.g., texture rendering, depth-only
    // rendering, HDR, deferred rendering) to be used in a graphics application.
    //
    struct FBO 
    {
        // Enum to define different types of framebuffers
        enum Type {
            Default = 0,       // Default framebuffer
            Texture,           // Standard texture framebuffer
            TextureHDR,        // High Dynamic Range (HDR) texture framebuffer
            TextureDepth,      // Depth-only texture framebuffer
            Deferred           // Framebuffer for deferred rendering
        } type; // Specifies the type of framebuffer

        GLuint handle;         // OpenGL handle for the framebuffer object

        GLuint channel;
        GLuint* pTexture;     // Pointer to the associated texture

        int width;            // Width of the framebuffer
        int height;           // Height of the framebuffer

        // Constructor to initialize the FBO with specific attributes
        FBO(Type type = Default, GLuint channel = GL_TEXTURE0, GLuint* pTexture = nullptr)
            : type{ type }, handle{ 0 }, channel{ channel }, pTexture{ pTexture }, width{ 0 }, height{ 0 }
        {
        }

        // Copy constructor
        FBO(const FBO& other)
            : type{ other.type }, handle{ 0 }, channel{ other.channel }, pTexture{ other.pTexture }, width{ 0 }, height{ 0 }
        {
        }

        void bind(int newwidth, int newheight);

        // Creates a texture buffer and attaches it to the FBO
        void createTextureBuffer(int width, int height, GLenum format, GLenum attachment, GLuint* pTexid = nullptr);

        // Creates a renderbuffer for depth storage and attaches it to the FBO
        void createDepthBuffer(int width, int height);

        // Configures the FBO for rendering to a color texture with a depth buffer
        void resetTextureAsTarget(int width, int height, GLenum format);

        void resetTextureDepthAsTarget(int width, int height);

        // Configures the FBO for deferred rendering
        void resetDeferredAsTarget(int width, int height);
    };
}
