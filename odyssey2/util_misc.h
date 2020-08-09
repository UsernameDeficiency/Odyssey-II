/* Miscellaneous utility functions for the main program */
#pragma once
#include <glad/glad.h>
#include <unordered_map>
#include <vector>

struct Terrain_heights
{
	float min_height{ FLT_MAX };
	float max_height{ -FLT_MAX };
	float sea_y_pos{};
};

typedef struct
{
    std::vector<GLfloat> vertexArray;
    std::vector<GLfloat> normalArray;
    std::vector<GLfloat> texCoordArray;
    std::vector<GLuint> indexArray;
    GLsizei numVertices;
    GLsizei numIndices;

    // VBO and VAO IDs
    GLuint vao;
    GLuint vb, ib, nb, tb; // VBOs
} Model;

// Keyboard state for controls (TODO?)
extern std::unordered_map<int, int> key_state;

/* Build Model from generated terrain. */
Model* generate_terrain(const unsigned int world_size, const float world_xz_scale, float tex_scale);

/* Load a cubemap texture. */
void load_cubemap();

/* Exits the program on unrecoverable error, printing an error string to stderr */
void exit_on_error(const char* error);
