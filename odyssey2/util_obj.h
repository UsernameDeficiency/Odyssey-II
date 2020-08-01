#pragma once
#include <glad/glad.h>

typedef struct
{
    GLfloat* vertexArray;
    GLfloat* normalArray;
    GLfloat* texCoordArray;
    GLuint* indexArray;
    int numVertices;
    int numIndices;

    // Space for saving VBO and VAO IDs
    GLuint vao; // VAO
    GLuint vb, ib, nb, tb; // VBOs
} Model;

// Extended, load model and upload to arrays!
// DrawModel is for drawing such preloaded models.
void DrawModel2(Model* m, GLuint program, const char* vertexVariableName, 
    const char* normalVariableName, const char* texCoordVariableName);

Model* LoadDataToModel2(GLfloat* vertices, GLfloat* normals,
    GLfloat* texCoords, GLuint* indices, int numVert, int numInd);
