/*!*****************************************************************************
\file CG/texture.cpp
\author Vadim Surov (vsurov@digipen.edu)
\date 12/23/2025 (MM/DD/YYYY)
\brief Texture implementation: loads 2D and HDR cubemap images via stb_image,
       creates OpenGL textures, and provides bind/cleanup utilities.
*******************************************************************************/
#include <stdexcept>
#include <sstream>   
#include <fstream>

#include "texture.h"

#include <glm/glm.hpp>

// For loading textures
#include "stb_image.h"
#include <GLFW/glfw3.h>

namespace
{
    // This function utilizes the stb_image library for loading image files and OpenGL 
    // for creating and handling the texture.
    //
    // Fix: enable vertical flip so textures match OpenGL's bottom-left origin.
    GLuint loadTexture(const char* filename)
    {
        int width = 0, height = 0, bytesPerPix = 0;

        // Flip vertically to correct rotation/inversion from image origin mismatch.
        stbi_set_flip_vertically_on_load(1);

        unsigned char* data = stbi_load(filename, &width, &height, &bytesPerPix, 4);
        
        // Reset flip state to default to avoid side-effects on other loads.
        stbi_set_flip_vertically_on_load(0);

        if (!data)
            throw std::runtime_error(std::string("file ") + filename + " not found: " + stbi_failure_reason());

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        // Save and set unpack alignment
        GLint prevAlign = 0;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevAlign);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // compute mip levels (guard against zero dimensions)
        int maxDim = std::max(width, height);
        int levels = (maxDim > 0) ? (1 + static_cast<int>(std::floor(std::log2(maxDim)))) : 1;
        glTexStorage2D(GL_TEXTURE_2D, levels, GL_RGBA8, width, height);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glGenerateMipmap(GL_TEXTURE_2D);

        // restore previous alignment
        glPixelStorei(GL_UNPACK_ALIGNMENT, prevAlign);

        stbi_image_free(data);

        return tex;
    }


    // The loadHdrCubeMap function generates and configures a cube map 
    // texture using HDR image files
    GLuint loadHdrCubeMap(const std::vector<std::string> filenames)
    {
        if (filenames.size() != 6)
            throw std::runtime_error("loadHdrCubeMap expects exactly 6 faces");

        GLuint texID = 0;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

        int faceIndex = 0;
        int w0 = 0, h0 = 0;
        for (const std::string& filename : filenames)
        {
            int w = 0, h = 0, channels = 0;
            float* data = stbi_loadf(filename.c_str(), &w, &h, &channels, 3);
            if (!data)
                throw std::runtime_error(std::string("file ") + filename + " not found: " + stbi_failure_reason());

            if (faceIndex == 0)
            {
                w0 = w; h0 = h;
                int maxDim = std::max(w0, h0);
                int levels = (maxDim > 0) ? (1 + static_cast<int>(std::floor(std::log2(maxDim)))) : 1;
                glTexStorage2D(GL_TEXTURE_CUBE_MAP, levels, GL_RGB32F, w0, h0);
            }
            else
            {
                if (w != w0 || h != h0)
                {
                    stbi_image_free(data);
                    throw std::runtime_error("All cube map faces must have the same dimensions");
                }
            }

            GLint prevAlign = 0;
            glGetIntegerv(GL_UNPACK_ALIGNMENT, &prevAlign);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, 0, 0, 0, w, h, GL_RGB, GL_FLOAT, data);
            glPixelStorei(GL_UNPACK_ALIGNMENT, prevAlign);

            stbi_image_free(data);
            ++faceIndex;
        }

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return texID;
    }
}

namespace cg
{
    // Constructors
    Texture::Texture(const std::initializer_list<const char*>& filenames, GLuint channel /*= GL_TEXTURE0*/)
        : filenames{ }, channel{ channel }, id{ 0 }, target{ 0 } //, key{ 0 }
    {
        this->filenames.reserve(filenames.size());
        for (auto s : filenames)  
            this->filenames.emplace_back(s ? s : "");
    }

    Texture::Texture(GLuint id /*= 0*/, GLuint channel /*= GL_TEXTURE0*/, GLuint target /*= GL_TEXTURE_2D*/)
        : filenames{ }, id{ id }, channel{ channel }, target{ target } 
    {
    }

    // Method to add and bind a new texture or bind an existing one
    void Texture::bind()
    {
        // First pass: ensure textures are loaded
        if (!id)
        {
            if (filenames.size() == 1)
            {
                id = loadTexture(filenames[0].c_str());
                target = GL_TEXTURE_2D;
            }
            else if (filenames.size() == 6)
            {
                id = loadHdrCubeMap(filenames);
                target = GL_TEXTURE_CUBE_MAP;
            }
        }

        if (id)
        {
            glActiveTexture(channel);
            glBindTexture(target, id);
        }
    }

    // Clean up all loaded textures
    void Texture::cleanup()
    {
        glDeleteTextures(1, &id);
        id = 0;
    }
}
