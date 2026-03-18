/*!*****************************************************************************
\file CG/fbo.h
\author Vadim Surov (vsurov@digipen.edu)
\date 03/14/2026 (MM/DD/YYYY)
\brief Framebuffer Object interface: declares FBO types and API to bind,
       allocate color/HDR/depth textures, and configure deferred targets.
*******************************************************************************/
#pragma once

#include <GL/glew.h>

namespace cg
{

    // Creates a texture buffer and attaches it to the FBO
    void createTextureBuffer(int width, int height,
        GLenum texUnit, GLuint* pTexid,
        GLenum format, GLenum attachment);

    // Creates a renderbuffer for depth storage and attaches it to the FBO
    void createDepthBuffer(int width, int height);

    //
    // This struct that sets up and manages OpenGL Framebuffer Objects (FBOs), allowing 
    // different rendering techniques(e.g., texture rendering, depth-only
    // rendering, HDR, deferred rendering) to be used in a graphics application.
    //
    struct FBO 
    {
        // Enum to define different types of framebuffers
        enum Type {
            Default = 0,    // Default framebuffer
            Texture,        // Standard texture framebuffer
            TextureHDR,     // High Dynamic Range (HDR) texture framebuffer
            TextureDepth,   // Depth-only texture framebuffer
            Deferred        // Framebuffer for deferred rendering
        } type; // Specifies the type of framebuffer

        GLuint handle;      // OpenGL handle for the framebuffer object

		GLuint texUnit0;    // Texture unit for channel 0 (e.g., GL_TEXTURE0)
        GLuint texUnit1;    // Texture unit for channel 1 (e.g., GL_TEXTURE1)
        GLuint texUnit2;    // Texture unit for channel 2 (e.g., GL_TEXTURE2)
        GLuint* pTexid0;    // Pointer to the associated texture for channel 0
        GLuint* pTexid1;    // Pointer to the associated texture for channel 1
        GLuint* pTexid2;    // Pointer to the associated texture for channel 2

        int width;          // Width of the framebuffer
        int height;         // Height of the framebuffer

        // Constructor to initialize the FBO with specific attributes
        FBO(Type type = Default, 
            GLuint texUnit0 = GL_TEXTURE0, GLuint* pTexid0 = nullptr,
            GLuint texUnit1 = GL_TEXTURE1, GLuint* pTexid1 = nullptr,
            GLuint texUnit2 = GL_TEXTURE2, GLuint* pTexid2 = nullptr)
            : type{ type }, handle{ 0 }, 
              texUnit0{ texUnit0 }, pTexid0{ pTexid0 }, 
              texUnit1{ texUnit1 }, pTexid1{ pTexid1 }, 
              texUnit2{ texUnit2 }, pTexid2{ pTexid2 }, 
              width{ 0 }, height{ 0 }
        {
        }

        // Copy constructor
        FBO(const FBO& other)
            : type{ other.type }, handle{ 0 }, 
              texUnit0{ other.texUnit0 }, pTexid0{ other.pTexid0 }, 
              texUnit1{ other.texUnit1 }, pTexid1{ other.pTexid1 }, 
              texUnit2{ other.texUnit2 }, pTexid2{ other.pTexid2 }, 
              width{ 0 }, height{ 0 }
        {
        }

        void bind(int newwidth, int newheight);

        // Configures the FBO for rendering to a color texture with a depth buffer
        void resetTextureAsTarget(int width, int height, GLenum format);

        void resetTextureDepthAsTarget(int width, int height);

        // Configures the FBO for deferred rendering
        void resetDeferredAsTarget(int width, int height);
    };
}
