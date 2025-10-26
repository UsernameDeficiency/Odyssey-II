#pragma once
#include <glad/glad.h>
#include <vector>

// Data container for terrain model
struct Model
{
	// Construct Model with full set of vectors
	Model(std::vector<GLfloat> vertex_array, const GLsizei num_vertices, const GLsizei num_indices)
		: vertex_array(std::move(vertex_array)), num_vertices(num_vertices), num_indices(num_indices),
		  vao(0), vb(0), ib(0), nb(0), tb(0)
	{
	}

	// Vertex data is needed on CPU for terrain collision, rest is GPU only
	std::vector<GLfloat> vertex_array;
	GLsizei num_vertices;
	GLsizei num_indices;

	// VBO and VAO IDs
	GLuint vao;
	GLuint vb, ib, nb, tb; // VBOs
};
