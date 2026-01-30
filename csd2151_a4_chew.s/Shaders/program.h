/*!*****************************************************************************
\file CG/program.h
\author Vadim Surov (vsurov@digipen.edu)
\date 01/18/2025 (MM/DD/YYYY)
\brief Shader program interface: declares API to compile/link GLSL shaders,
       manage uniforms, bind locations, and validate/introspect programs.
*******************************************************************************/
#pragma once

// Include necessary libraries for string manipulation, OpenGL, and glm (mathematics)
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <glm/glm.hpp>
#include <GL/glew.h>

namespace cg
{
    //
    // Shader Program class for managing shader program handle and operations.
    //
    class Program
    {
    public:
        GLuint handle; // OpenGL shader program handle
        const char* sc;
        const char* sv;
        const char* sf;

        bool linked; // Indicates if the shader program is successfully linked

    private:
        std::map<std::string, int> uniformLocations; // Cache for uniform variable locations

        // Helper function to retrieve the location of a uniform variable by name
        GLint getUniformLocation(const std::string& name)
        {
            // Search for the uniform location in the cache
            auto pos = uniformLocations.find(name);
            if (pos == uniformLocations.end())
                // If not found, get the location from OpenGL and store it in the cache
                return uniformLocations[name] = glGetUniformLocation(handle, name.c_str());
            return pos->second; // Return cached location if found
        }

        // Link the shader program
        void link();                         

        // Detach and delete shader objects
        void detachAndDeleteShaderObjects(); 

    public:
        // Constructor that takes a vertex and a fragment shaders
        Program(const char* sv, const char* sf)
            : sc{ nullptr }, sv{ sv }, sf{ sf }, handle{ 0 }, linked{ false }, uniformLocations{ }
        {
        }

        // Constructor that takes a compute shaders
        Program(const char* sc)
            : sc{ sc }, sv{ nullptr }, sf{ nullptr }, handle{ 0 }, linked{ false }, uniformLocations{ }
        {
        }

        // Defaut constructor
        Program()
            : sc{ nullptr }, sv{ nullptr }, sf{ nullptr }, handle{ 0 }, linked{ false }, uniformLocations{ }
        {
        }

		// Copy constructor
        Program(const Program& program)
            : sc{ program.sc }, sv{ program.sv }, sf{ program.sf }, handle{ 0 }, linked{ false }, uniformLocations{ }
        {
		}

		// Copy assignment operator
        Program& operator=(const Program& program) = delete;

        // Destructor
        ~Program(); 

        // Compile a shader
        void compileShader(const char* code, GLenum type);

		// Build (compile and link) the shader program
        void build();

		// Clean up resources associated with the shader program
		void clean();

        // Convert the Program object to its OpenGL handle for easier use in OpenGL functions

		// Validate the program (ensure it works correctly)
        void validate(); 

        // Activate the shader program for use
        void use() const; 

        // Deactivate the shader program
        void disuse() const; 

		// Implicit conversion to GLuint (OpenGL program handle)
        operator GLuint() {
            return handle;
        }

        // Bind an attribute location (used to associate vertex data with shader inputs)
        void bindAttribLocation(GLuint location, const char* name) {
            glBindAttribLocation(handle, location, name);
        }

        // Bind a fragment data location (for output of the fragment shader)
        void bindFragDataLocation(GLuint location, const char* name) {
            glBindFragDataLocation(handle, location, name);
        }

        // Set a uniform variable of type vec3 (float x, y, z)
        void setUniform(const std::string& name, float x, float y, float z) {
            glUniform3f(getUniformLocation(name), x, y, z);
        }

        // Set a uniform variable of type vec2 (glm::vec2)
        void setUniform(const std::string& name, const glm::vec2& v) {
            glUniform2f(getUniformLocation(name), v.x, v.y);
        }

        // Set a uniform variable of type vec3 (glm::vec3)
        void setUniform(const std::string& name, const glm::vec3& v) {
            this->setUniform(name, v.x, v.y, v.z);
        }

        // Set a uniform variable of type vec4 (glm::vec4)
        void setUniform(const std::string& name, const glm::vec4& v) {
            glUniform4f(getUniformLocation(name), v.x, v.y, v.z, v.w);
        }

        // Set a uniform variable for vector types (std::vector<float>)
        void setUniform(const std::string& name, const std::vector<float>& v) {
            if (v.size() == 1)
                glUniform1f(getUniformLocation(name), v[0]);
            else if (v.size() == 2)
                glUniform2f(getUniformLocation(name), v[0], v[1]);
            else if (v.size() == 3)
                glUniform3f(getUniformLocation(name), v[0], v[1], v[2]);
            else if (v.size() == 4)
                glUniform4f(getUniformLocation(name), v[0], v[1], v[2], v[3]);
        }

        // Set a uniform matrix4fv (4x4 matrix, typically used for transformations)
        void setUniform(const std::string& name, const glm::mat4& m) {
            glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &m[0][0]);
        }

        // Set a uniform matrix3fv (3x3 matrix)
        void setUniform(const std::string& name, const glm::mat3& m) {
            glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &m[0][0]);
        }

        // Set a uniform float variable
        void setUniform(const std::string& name, float val) {
            glUniform1f(getUniformLocation(name), val);
        }

        // Set a uniform integer variable
        void setUniform(const std::string& name, int val) {
            glUniform1i(getUniformLocation(name), val);
        }

        // Set a uniform boolean variable (converted to int)
        void setUniform(const std::string& name, bool val) {
            glUniform1i(getUniformLocation(name), val);
        }

        // Set a uniform unsigned integer variable
        void setUniform(const std::string& name, GLuint val) {
            glUniform1ui(getUniformLocation(name), val);
        }

		// Set a uniform array of unsigned integers
        void setUniform(const std::string& name, GLsizei count, const GLuint* value) {
            glUniform1uiv(getUniformLocation(name), count, value);
        }

		// Find and cache all uniform locations in the shader program
        void findUniformLocations(); 

        // Debugging utility
        void printActiveUniforms();

        // Debugging utility
        void printActiveUniformBlocks();

		// Debugging utility
        void printActiveAttribs();
    };
}
