/*!*****************************************************************************
\file CG/texture.h
\author Vadim Surov (vsurov@digipen.edu)
\date 12/23/2025 (MM/DD/YYYY)
\brief Texture interface: declares a texture wrapper to load/bind/cleanup 2D and
       cubemap textures with channel/target management.
*******************************************************************************/
#pragma once

#include <vector>
#include <string>
#include <initializer_list>

#include <GL/glew.h>

namespace cg 
{
    // Structure to represent a texture used in rendering
    struct Texture
    {
        std::vector<std::string> filenames; // Member to store the initialized data
        GLuint channel;                     // Texture channel (e.g., GL_TEXTURE0)
        GLuint id;                          // Texture ID assigned by OpenGL
        GLuint target;                      // Texture target (e.g., GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP)

        // Constructors
        Texture(const std::initializer_list<const char*>& filenames, GLuint channel = GL_TEXTURE0);

        Texture(GLuint id=0, GLuint channel = GL_TEXTURE0, GLuint target = GL_TEXTURE_2D);

        // Method to add and bind a new texture or bind an existing one
        void bind();

        // Clean up 
        void cleanup();
    };
}
