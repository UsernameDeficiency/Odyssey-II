#pragma once
#include <vector>
#include <glad/glad.h>

// TODO: Potentially move Model to its own file
/* Holds data for terrain model */
struct Model
{
    // Construct Model with full set of vectors
    Model(std::vector<GLfloat> vertexArray, GLsizei numVertices, GLsizei numIndices)
        : vertexArray(std::move(vertexArray)), numVertices(numVertices), numIndices(numIndices),
        vao(0), vb(0), ib(0), nb(0), tb(0)
    { }

    // Vertex data is needed on CPU for terrain collision, rest is GPU only
    std::vector<GLfloat> vertexArray;
    GLsizei numVertices;
    GLsizei numIndices;

    // VBO and VAO IDs
    GLuint vao;
    GLuint vb, ib, nb, tb; // VBOs
};
