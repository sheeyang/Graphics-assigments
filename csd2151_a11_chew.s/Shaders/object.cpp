/*!*****************************************************************************
\file CG/object.cpp
\author Vadim Surov (vsurov@digipen.edu)
\date 12/23/2025 (MM/DD/YYYY)
\brief Object implementation: builds VAO/VBOs from mesh data (indices, positions,
       normals, UVs, tangents) and configures vertex attributes for rendering.
*******************************************************************************/
#include "object.h"
#include "trianglemeshes.h"

namespace cg
{
    // Move constructor
    Object::Object(Object&& other) noexcept
        : meshName{ other.meshName }
        , vao{ other.vao }
        , buffers{ std::move(other.buffers) }
        , size{ other.size }
        , instances{ std::move(other.instances) }
    {
        other.vao = 0;
        other.size = 0;
    }

    // Move assignment
    Object& Object::operator=(Object&& other) noexcept
    {
        if (this != &other)
        {
            release();

            meshName = other.meshName;
            vao = other.vao;
            buffers = std::move(other.buffers);
            size = other.size;
            instances = std::move(other.instances);

            other.vao = 0;
            other.size = 0;
        }
        return *this;
    }

    // Destructor
    Object::~Object()
    {
        release();
    }

    // Explicit resource release
    void Object::release()
    {
        if (!buffers.empty())
        {
            glDeleteBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
            buffers.clear();
        }

        if (vao != 0)
        {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }

        size = 0;
    }

    // Setting up the necessary OpenGL buffers and vertex attributes for a 3D object
    void Object::bind()
    {
        // Prevent duplicate allocations if already bound
        if (vao != 0)
            return;

        // Load the mesh data (vertices, indices, normals, etc.) from the given mesh file
        cg::TriangleMesh mesh(meshName);

        // Check if the mesh has valid data for indices, points (vertices), and normals.
        if (mesh.indices.empty() || mesh.points.empty() || mesh.normals.empty())
            return;

        // Store the number of indices.
        size = mesh.indices.size();

        // Prepare to store buffer IDs
        buffers.reserve(5);

        // Declare OpenGL buffer object IDs for indices, positions, normals, texCoords, and tangents
        GLuint indexBuf = 0, posBuf = 0, normBuf = 0, tcBuf = 0, tangentBuf = 0;

        // Create and bind a buffer for the index data (element array buffer)
        glGenBuffers(1, &indexBuf);
        buffers.push_back(indexBuf);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.indices.size() * sizeof(GLuint)), mesh.indices.data(), GL_STATIC_DRAW);

        // Create and bind a buffer for the vertex positions (array buffer)
        glGenBuffers(1, &posBuf);
        buffers.push_back(posBuf);
        glBindBuffer(GL_ARRAY_BUFFER, posBuf);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.points.size() * sizeof(GLfloat)), mesh.points.data(), GL_STATIC_DRAW);

        // Create and bind a buffer for the normal data (array buffer)
        glGenBuffers(1, &normBuf);
        buffers.push_back(normBuf);
        glBindBuffer(GL_ARRAY_BUFFER, normBuf);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.normals.size() * sizeof(GLfloat)), mesh.normals.data(), GL_STATIC_DRAW);

        // If the mesh contains texture coordinates, create and bind a buffer for them
        if (!mesh.texCoords.empty())
        {
            glGenBuffers(1, &tcBuf);
            buffers.push_back(tcBuf);
            glBindBuffer(GL_ARRAY_BUFFER, tcBuf);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.texCoords.size() * sizeof(GLfloat)), mesh.texCoords.data(), GL_STATIC_DRAW);
        }

        // If the mesh contains tangent data, create and bind a buffer for tangents
        if (!mesh.tangents.empty())
        {
            glGenBuffers(1, &tangentBuf);
            buffers.push_back(tangentBuf);
            glBindBuffer(GL_ARRAY_BUFFER, tangentBuf);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(mesh.tangents.size() * sizeof(GLfloat)), mesh.tangents.data(), GL_STATIC_DRAW);
        }

        // Generate and bind a Vertex Array Object (VAO) to store the vertex attribute bindings
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Bind the index buffer (element array buffer) for use in drawing
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

        // Set up the vertex position attribute (3D coordinates: x, y, z)
        glBindBuffer(GL_ARRAY_BUFFER, posBuf);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        // Set up the vertex normal attribute (3D normal vector: nx, ny, nz)
        glBindBuffer(GL_ARRAY_BUFFER, normBuf);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        // If texture coordinates are available, set up the texture coordinate attribute (2D: u, v)
        if (!mesh.texCoords.empty())
        {
            glBindBuffer(GL_ARRAY_BUFFER, tcBuf);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(2);
        }

        // If tangents are available, set up the tangent attribute (4D vector: tx, ty, tz, tw)
        if (!mesh.tangents.empty())
        {
            glBindBuffer(GL_ARRAY_BUFFER, tangentBuf);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(3);
        }

        // Unbind the VAO to finish setting up the vertex buffers and attributes
        glBindVertexArray(0);
    }
}