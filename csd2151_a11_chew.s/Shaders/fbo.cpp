/*!*****************************************************************************
\file CG/fbo.cpp
\author Vadim Surov (vsurov@digipen.edu)
\date 03/15/2026 (MM/DD/YYYY)
\brief Framebuffer Object implementation: creates/binds FBOs, allocates and
       attaches color/HDR/depth textures, and configures deferred render targets.
*******************************************************************************/
#include "fbo.h"

#include <stdexcept>
#include <string>

namespace cg
{
    void FBO::bind(int newwidth, int newheight)
    {
        switch (type)
        {
        case Default:
            glBindFramebuffer(GL_FRAMEBUFFER, handle);
            break; // No additional setup for default type

        case Texture: // Setup for color texture
            resetTextureAsTarget(newwidth, newheight, GL_RGBA8);
            break;

        case TextureHDR: // Setup for HDR texture
            resetTextureAsTarget(newwidth, newheight, GL_RGBA16F);
            break;

        case TextureDepth: // Setup for depth texture
            resetTextureDepthAsTarget(newwidth, newheight);
            break;

        case Deferred: // Setup for deferred rendering
            resetDeferredAsTarget(newwidth, newheight); 
            break;
        }
    }

    // Creates a texture buffer and attaches it to the FBO
    void createTextureBuffer(int width, int height, 
        GLenum texUnit, GLuint* pTexid,
        GLenum format, GLenum attachment)
    {
        GLuint texid;

        // Activate the specified texture unit and generate a texture
        glActiveTexture(texUnit);
        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);

        // Create storage using matching external format/type
		GLenum type = (format == GL_RGB8) ? GL_UNSIGNED_BYTE : GL_FLOAT;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, type, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        // Attach the texture to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texid, 0);

        if (pTexid)
            *pTexid = texid; // Return texture ID if a pointer is provided
    }

    // Creates a renderbuffer for depth storage and attaches it to the FBO
    void createDepthBuffer(int width, int height)
    {
        GLuint depthBuf;
        glGenRenderbuffers(1, &depthBuf);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

        // Attach the depth buffer to the framebuffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
    }

    // Configures the FBO for rendering to a color texture with a depth buffer
    void FBO::resetTextureAsTarget(int width, int height, GLenum format)
    {
        // Create and bind the framebuffer
        if (handle == 0)
            glGenFramebuffers(1, &handle);
        glBindFramebuffer(GL_FRAMEBUFFER, handle);

        if (this->width != width || this->height != height)
        {
            this->width = width;
            this->height = height;

            // Create a color buffer texture and attach it
            createTextureBuffer(width, height, texUnit0, pTexid0, format, GL_COLOR_ATTACHMENT0);

            // Create and attach a depth buffer
            createDepthBuffer(width, height);

            // Specify the draw buffers
            GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, drawBuffers);

            // Check for errors
            GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (result != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error(std::string("Error ") +
                    std::to_string(result) +
                    ": Framebuffer is not complete.\n");
        }
    }

    void FBO::resetTextureDepthAsTarget(int width, int height)
    {
        // Create and bind the framebuffer
        if (handle == 0)
            glGenFramebuffers(1, &handle);
        glBindFramebuffer(GL_FRAMEBUFFER, handle);

        if (this->width != width || this->height != height)
        {
            this->width = width;
            this->height = height;

            GLuint texture;
            GLuint* tex = pTexid0 ? pTexid0 : &texture;

            // If caller provided storage for the texture id, remember the old one for cleanup
            GLuint old = (pTexid0 && *pTexid0 != 0) ? *pTexid0 : 0;
                
            // The depth buffer texture
            glGenTextures(1, tex);
            glBindTexture(GL_TEXTURE_2D, *tex);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, width, height);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            GLfloat border[] = { 1.0f, 0.0f,0.0f,0.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *tex, 0);

            // Disable color writes/reads for a depth-only FBO
            GLenum drawBuffers[] = { GL_NONE };
            glDrawBuffers(1, drawBuffers);
            glReadBuffer(GL_NONE);

            // Clean up previously allocated texture (if we own/manage the id via pTexture)
            if (old != 0)
                glDeleteTextures(1, &old);

            // Check for errors
            GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (result != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error(std::string("Error ") +
                    std::to_string(result) +
                    ": Framebuffer is not complete.\n");
        }
    }

    // Configures the FBO for deferred rendering
    void FBO::resetDeferredAsTarget(int width, int height)
    {
        // Create and bind the framebuffer
        if (handle == 0)
            glGenFramebuffers(1, &handle);
        glBindFramebuffer(GL_FRAMEBUFFER, handle);

        if (this->width != width || this->height != height)
        {
            this->width = width;
            this->height = height;

            // Create textures for different deferred rendering passes
            createTextureBuffer(width, height, texUnit0, pTexid0, GL_RGB32F, GL_COLOR_ATTACHMENT0);
            createTextureBuffer(width, height, texUnit1, pTexid1, GL_RGB32F, GL_COLOR_ATTACHMENT1);
            createTextureBuffer(width, height, texUnit2, pTexid2, GL_RGB8,   GL_COLOR_ATTACHMENT2);

            // Define an array of buffers into which outputs from 
            // the fragment shader data will be written
            GLenum drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
            glDrawBuffers(4, drawBuffers);

            // Create and attach a depth buffer
            createDepthBuffer(width, height);

            // Check for errors
            GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (result != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error(std::string("Error ") + std::to_string(result) +
                    ": Framebuffer is not complete.\n");
        }
    }
}
