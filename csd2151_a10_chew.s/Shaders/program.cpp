/*!*******************************************************************************
\file CG/program.cpp
\author Vadim Surov (vsurov@digipen.edu)
\date 12/26/2024 (MM/DD/YYYY)
\brief Shader program implementation: compiles/links GLSL shaders, manages uniform
       locations, and provides introspection/validation utilities.
*******************************************************************************/
#include <sstream>
#include <string>
#include <vector>
#include <cassert>

#include "program.h"

namespace
{
    // Function to convert OpenGL shader data type enums to strings
    const char* getTypeString(GLenum type)
    {
        switch (type) {
        case GL_FLOAT:        return "float";
        case GL_FLOAT_VEC2:   return "vec2";
        case GL_FLOAT_VEC3:   return "vec3";
        case GL_FLOAT_VEC4:   return "vec4";
        case GL_DOUBLE:       return "double";
        case GL_INT:          return "int";
        case GL_UNSIGNED_INT: return "unsigned int";
        case GL_BOOL:         return "bool";
        case GL_FLOAT_MAT2:   return "mat2";
        case GL_FLOAT_MAT3:   return "mat3";
        case GL_FLOAT_MAT4:   return "mat4";
        default:              return "?";
        }
    }

    const char* getStageString(GLenum type)
    {
        switch (type) {
        case GL_VERTEX_SHADER:   return "vertex";
        case GL_FRAGMENT_SHADER: return "fragment";
        case GL_COMPUTE_SHADER:  return "compute";
        case GL_GEOMETRY_SHADER: return "geometry";
        case GL_TESS_CONTROL_SHADER: return "tess_control";
        case GL_TESS_EVALUATION_SHADER: return "tess_evaluation";
        default: return "unknown";
        }
    }
}

namespace cg
{

    // Build (compile and link) the shader program
    void Program::build()
    {
        // If rebuilding, clean previous GL objects
        if (handle != 0)
            clean();

        if (sv && sf && strlen(sv) && strlen(sf))
        {
            compileShader(sv, GL_VERTEX_SHADER);
            compileShader(sf, GL_FRAGMENT_SHADER);
            link();
        }
        else if (sc && strlen(sc))
        {
            compileShader(sc, GL_COMPUTE_SHADER);
            link();
        }
    }

    // Clean up resources associated with the shader program
    void Program::clean()
    {
        if (!handle)
            return;

        // Detach and delete shaders
        detachAndDeleteShaderObjects();

        // Delete the shader program
        glDeleteProgram(handle);

        // Reset state
        handle = 0;
        linked = false;
        uniformLocations.clear();
    }

    // Destructor: cleans up by detaching and deleting shader objects, and deleting the program
    Program::~Program()
    {
        clean();
    }

    // Detach and delete shader objects from the program
    void Program::detachAndDeleteShaderObjects()
    {
        if (!handle)
            return;

        GLint numShaders = 0;
        glGetProgramiv(handle, GL_ATTACHED_SHADERS, &numShaders);
        std::vector<GLuint> shaderNames(static_cast<size_t>(numShaders));
        glGetAttachedShaders(handle, numShaders, NULL, shaderNames.data());
        for (GLuint shader : shaderNames)
        {
            glDetachShader(handle, shader);
            glDeleteShader(shader);
        }
    }

    // Compile a shader
    void Program::compileShader(const char* code, GLenum type)
    {
        if (!handle)
        {
            handle = glCreateProgram();
            if (!handle)
                throw std::runtime_error("Unable to create shader program.");
        }

        GLuint shaderHandle = glCreateShader(type);
        glShaderSource(shaderHandle, 1, &code, nullptr);
        glCompileShader(shaderHandle);

        GLint result = GL_FALSE;
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            std::string msg = "Shader compilation failed";
            msg += " [" + std::string(getStageString(type)) + "].\n";

            GLint length = 0;
            glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
            if (length > 1)
            {
                std::vector<char> log(static_cast<size_t>(length));
                GLsizei written = 0;
                glGetShaderInfoLog(shaderHandle, length, &written, log.data());
                msg.append(log.data(), static_cast<size_t>(written));
            }

            // Delete the failed shader object to avoid leaks
            glDeleteShader(shaderHandle);
            throw std::runtime_error(msg);
        }

        glAttachShader(handle, shaderHandle);
    }

    // Link the program after shaders have been attached
    void Program::link()
    {
        assert(handle && "Program::link(): program handle is 0");

        glLinkProgram(handle);
        GLint status = GL_FALSE;
        glGetProgramiv(handle, GL_LINK_STATUS, &status);

        std::string errString;
        if (status == GL_FALSE)
        {
            GLint length = 0;
            glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);
            errString += "Program link failed:\n";
            if (length > 1)
            {
                std::vector<char> log(static_cast<size_t>(length));
                GLsizei written = 0;
                glGetProgramInfoLog(handle, length, &written, log.data());
                errString.append(log.data(), static_cast<size_t>(written));
            }
        }
        else
        {
            linked = true;
            findUniformLocations();
        }

        // After linking, shader objects are no longer needed; detach & delete
        detachAndDeleteShaderObjects();

        if (status == GL_FALSE)
            throw std::runtime_error(errString);
    }

    // Find and store the locations of all active uniforms in the program
    void Program::findUniformLocations()
    {
        assert(handle && "Program::findUniformLocations(): handle is 0");

        uniformLocations.clear();

        GLint numUniforms = 0;
        glGetProgramInterfaceiv(handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

        const GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };

        for (GLint i = 0; i < numUniforms; ++i)
        {
            GLint results[4] = { 0, 0, -1, -1 };
            glGetProgramResourceiv(handle, GL_UNIFORM, i, 4, properties, 4, NULL, results);

            // Skip uniforms inside blocks (block index != -1)
            if (results[3] != static_cast<GLint>(-1))
                continue;

            // Skip uniforms with invalid locations (-1)
            if (results[2] == static_cast<GLint>(-1))
                continue;

            const GLint nameBufSize = results[0] + 1;
            std::vector<char> name(static_cast<size_t>(nameBufSize));
            glGetProgramResourceName(handle, GL_UNIFORM, i, nameBufSize, NULL, name.data());

            uniformLocations[std::string(name.data())] = results[2];
        }
    }

    // Set the shader program as the current active program
    void Program::use() const
    {
        // Debug-time assertions: ensure program is ready to be used
        assert(handle && "Program::use(): program handle is 0");
        assert(linked && "Program::use(): program is not linked");

        glUseProgram(handle);  
    }

    // Disable the use of the current shader program (use the default)
    void Program::disuse() const
    {
        glUseProgram(0);
    }

    // Print all active uniforms in the program to the console
    void Program::printActiveUniforms()
    {
        assert(handle && "Program::printActiveUniforms(): handle is 0");
        assert(linked && "Program::printActiveUniforms(): program is not linked");

        GLint numUniforms = 0;
        glGetProgramInterfaceiv(handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms); // Get number of active uniforms

        const GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };  // Properties to query for each uniform

        printf("Active uniforms:\n");
        for (GLint i = 0; i < numUniforms; ++i) {
            GLint results[4] = { 0, 0, -1, -1 };
            glGetProgramResourceiv(handle, GL_UNIFORM, i, 4, properties, 4, NULL, results);

            if (results[3] != static_cast<GLint>(-1))
                continue;

            const GLint nameBufSize = results[0] + 1;
            std::vector<char> name(static_cast<size_t>(nameBufSize));
            glGetProgramResourceName(handle, GL_UNIFORM, i, nameBufSize, NULL, name.data());

            printf("%-5d %s (%s)\n", results[2], name.data(), getTypeString(results[1]));
        }
    }

    // Print all active uniform blocks in the program to the console
    void Program::printActiveUniformBlocks()
    {
        assert(handle && "Program::printActiveUniformBlocks(): handle is 0");
        assert(linked && "Program::printActiveUniformBlocks(): program is not linked");

        GLint numBlocks = 0;

        glGetProgramInterfaceiv(handle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);  // Get number of uniform blocks
        
        const GLenum blockProps[] = { GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH };
        const GLenum blockIndex[] = { GL_ACTIVE_VARIABLES };
        const GLenum props[] = { GL_NAME_LENGTH, GL_TYPE, GL_BLOCK_INDEX };

        for (GLint block = 0; block < numBlocks; ++block) {
            GLint blockInfo[2] = { 0, 0 };
            glGetProgramResourceiv(handle, GL_UNIFORM_BLOCK, block, 2, blockProps, 2, NULL, blockInfo);
            const GLint numUnis = blockInfo[0];

            std::vector<char> blockName(static_cast<size_t>(blockInfo[1] + 1));
            glGetProgramResourceName(handle, GL_UNIFORM_BLOCK, block, blockInfo[1] + 1, NULL, blockName.data());
            printf("Uniform block \"%s\":\n", blockName.data());

            std::vector<GLint> unifIndexes(static_cast<size_t>(numUnis));
            glGetProgramResourceiv(handle, GL_UNIFORM_BLOCK, block, 1, blockIndex, numUnis, NULL, unifIndexes.data());

            for (GLint unif = 0; unif < numUnis; ++unif) {
                const GLint uniIndex = unifIndexes[unif];
                GLint results[3] = { 0, 0, -1 };
                glGetProgramResourceiv(handle, GL_UNIFORM, uniIndex, 3, props, 3, NULL, results);

                const GLint nameBufSize = results[0] + 1;
                std::vector<char> name(static_cast<size_t>(nameBufSize));
                glGetProgramResourceName(handle, GL_UNIFORM, uniIndex, nameBufSize, NULL, name.data());

                printf("    %s (%s)\n", name.data(), getTypeString(results[1]));
            }
        }
    }

    // Print all active attributes in the program to the console
    void Program::printActiveAttribs()
    {
        assert(handle && "Program::printActiveAttribs(): handle is 0");
        assert(linked && "Program::printActiveAttribs(): program is not linked");

        GLint numAttribs = 0;
        glGetProgramInterfaceiv(handle, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);

        const GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };

        printf("Active attributes:\n");
        for (GLint i = 0; i < numAttribs; ++i) {
            GLint results[3] = { 0, 0, -1 };
            glGetProgramResourceiv(handle, GL_PROGRAM_INPUT, i, 3, properties, 3, NULL, results);

            const GLint nameBufSize = results[0] + 1;
            std::vector<char> name(static_cast<size_t>(nameBufSize));
            glGetProgramResourceName(handle, GL_PROGRAM_INPUT, i, nameBufSize, NULL, name.data());

            printf("%-5d %s (%s)\n", results[2], name.data(), getTypeString(results[1]));
        }
    }

    // Validate the shader program by checking its correctness
    void Program::validate()
    {
        // Debug-time assertion: must be linked before validation
        assert(linked && "Program::validate(): program is not linked");

        GLint status;
        glValidateProgram(handle);  // Validate the shader program.
        glGetProgramiv(handle, GL_VALIDATE_STATUS, &status);  // Get validation status

        if (GL_FALSE == status) {  // If validation fails, retrieve and throw error message
            int length = 0;
            std::string logString;

            glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);  // Get error log length

            if (length > 0) {
                char* c_log = new char[length];
                int written = 0;
                glGetProgramInfoLog(handle, length, &written, c_log);
                logString = c_log;
                delete[] c_log;
            }

            throw std::runtime_error(std::string("Program failed to validate\n") + logString);
        }
    }
}