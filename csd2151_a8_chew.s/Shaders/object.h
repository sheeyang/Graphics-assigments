/*!*****************************************************************************
\file CG/object.h
\author Vadim Surov (vsurov@digipen.edu)
\date 12/23/2025 (MM/DD/YYYY)
\brief Object interface: declares a scene object with mesh, instances, and a bind
       method for VAO/VBO setup; includes macros for common demo object presets.
*******************************************************************************/
#pragma once

#include <vector>
#include <initializer_list>
#include <glm/glm.hpp>
#include <GL/glew.h>

#include "material.h"


#define NOTRANSFORM  glm::mat4(1.0f)

// Visibility and state controls
#define VISIBLE     true
#define INVISIBLE   false
#define ENABLE      true
#define DISABLE     false

#define DEMOOBJECTS_A8 \
    {\
        {\
            PLANE,\
            MATERIAL_CHECKERBOARD,\
            glm::translate(glm::scale(glm::mat4(1.0f), {3.0f, 1.0f, 3.0f}), {0.0f, -1.1f, 0.0f}),\
            VISIBLE\
        },\
        {\
            SPHERE,\
            MATERIAL_PBR_PHONG,\
            glm::translate(glm::mat4(1.0f), {2.5f, 0.0f, 0.0f}),\
            VISIBLE\
        },\
        {\
            TORUS,\
            MATERIAL_CARTOON,\
            NOTRANSFORM,\
            VISIBLE\
        },\
        {\
            TEAPOT,\
            MATERIAL_DISCARD,\
            glm::translate(glm::mat4(1.0f), {-2.5f, 0.0f, 0.0f}),\
            VISIBLE\
        }\
    }

#define DEMOOBJECTS_ENVIRONMENTMAPPING \
    {\
        {\
            SPHERE,\
            MATERIAL_REFLECT_REFRACT,\
            NOTRANSFORM,\
            VISIBLE\
        },\
        {\
            TORUS,\
            MATERIAL_REFLECT_REFRACT,\
            NOTRANSFORM,\
            INVISIBLE\
        },\
        {\
            CUBE,\
            MATERIAL_REFLECT_REFRACT,\
            NOTRANSFORM,\
            INVISIBLE\
        },\
        {\
            TEAPOT,\
            MATERIAL_REFLECT_REFRACT,\
            NOTRANSFORM,\
            INVISIBLE\
        }\
    }

#define DEMOOBJECTS \
    {\
        {\
            PLANE,\
            MATERIAL_PLANE,\
            glm::translate(glm::mat4(1.0f), { 0.0f, -1.1f, 0.0f }),\
            VISIBLE\
        },\
        {\
            TORUS,\
            MATERIAL_OBJECT,\
            NOTRANSFORM,\
            VISIBLE\
        },\
        {\
            CUBE,\
            MATERIAL_OBJECT,\
            NOTRANSFORM,\
            INVISIBLE\
        },\
        {\
            SPHERE,\
            MATERIAL_OBJECT,\
            NOTRANSFORM,\
            INVISIBLE\
        },\
        {\
            TEAPOT,\
            MATERIAL_OBJECT,\
            NOTRANSFORM,\
            INVISIBLE\
        }\
    }

#define DEMOOBJECTS_PBR \
    {\
        {\
            PLANE,\
            MATERIAL_PBR_PHONG,\
            glm::translate(glm::mat4(1.0f), { 0.0f, -1.1f, 0.0f }),\
            VISIBLE\
        },\
        {\
            TORUS,\
            MATERIAL_PBR_PHONG,\
            NOTRANSFORM,\
            VISIBLE\
        },\
        {\
            CUBE,\
            MATERIAL_PBR_PHONG,\
            NOTRANSFORM,\
            INVISIBLE\
        },\
        {\
            SPHERE,\
            MATERIAL_PBR_PHONG,\
            NOTRANSFORM,\
            INVISIBLE\
        },\
        {\
            TEAPOT,\
            MATERIAL_PBR_PHONG,\
            NOTRANSFORM,\
            INVISIBLE\
        }\
    }

#define DEMOOBJECTS_TONEMAPPING \
    {\
        {\
            PLANE,\
            MATERIAL_PLANE,\
            glm::translate(glm::mat4(1.0f), { 0.0f, -4.0f, 0.0f })\
        },\
        {\
            PLANE,\
            MATERIAL_PLANE,\
            glm::translate(glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 2, {1.0f, 0.0f, 0.0f}), {0.0f, -4.0f, 0.0f})\
        },\
        {\
            PLANE,\
            MATERIAL_PLANE,\
            glm::translate(glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 2, {0.0f, 0.0f, 1.0f}), {0.0f, -4.0f, 0.0f})\
        },\
        {\
            PLANE,\
            MATERIAL_PLANE,\
            glm::translate(glm::rotate(glm::mat4(1.0f), -glm::pi<float>() / 2, {0.0f, 0.0f, 1.0f}), {0.0f, -4.0f, 0.0f})\
        },\
        {\
            TEAPOT,\
            MATERIAL_OBJECT\
        }\
    }

namespace cg
{

    // Structure representing an object in the scene
    struct Object
    {
        const char* meshName;

        // Represents an instance of an object with its material and transform
        struct Instance
        {
            cg::Material material;       // Material properties of the instance
            glm::mat4 modeltransform;    // Model transformation matrix
            bool isVisible;              // Visibility flag

            // Constructor for creating an instance
            Instance(const cg::Material& material = { }, const glm::mat4& modeltransform = NOTRANSFORM, bool isVisible = VISIBLE)
                : material{ material }, modeltransform{ modeltransform }, isVisible{ isVisible }
            {
            }
        };

        GLuint vao;                         // Vertex Array Object ID for the object
        std::vector<GLuint> buffers;        // Vertex buffer objects
        size_t size;                        // Number of indices in the object's index buffer
        std::vector<Instance> instances;    // Instances of the object

        // Constructor for creating an object with a single instance
        Object(const char* meshName, const cg::Material& material = { },
            const glm::mat4& modeltransform = NOTRANSFORM, bool isVisible = VISIBLE)
            : meshName{ meshName }, vao{ 0 }, buffers{ }, size{ 0 }, instances{ {material, modeltransform, isVisible } }
        {
        }

        // Constructor for creating an object with multiple instances
        Object(const char* meshName,
            const std::initializer_list<Instance>& instances, bool isVisible = VISIBLE)
            : meshName{ meshName }, vao{ 0 }, buffers{ }, size{ 0 }, instances{ instances }
        {
        }

        // Copy constructor
        Object(const Object& object)
            : meshName{ object.meshName }, vao{ 0 }, buffers{ }, size{ 0 }, instances{ object.instances }
        {
        }

        // Move-only semantics to manage GL resources safely
        Object(Object&& other) noexcept;
        Object& operator=(Object&& other) noexcept;

        ~Object();

        // Free VAO/VBOs if allocated
        void release();

        // True if VAO/VBOs were created
        bool isBound() const { return vao != 0; }

        void bind();
    };
}
