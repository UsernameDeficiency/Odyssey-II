/* Miscellaneous utility functions for the main program */
#include "util_misc.h"
#include <iostream>
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "terrain.h"

extern struct Terrain_heights terrain_struct; // Used by generate_terrain to set heights for water and snow


/* Construct Model with full set of vectors, std::move all vectors */
Model::Model(std::vector<GLfloat> vertexArray, std::vector<GLfloat> normalArray, 
	std::vector<GLfloat> texCoordArray, std::vector<GLuint> indexArray,
	GLsizei numVertices, GLsizei numIndices)
	: vertexArray(std::move(vertexArray)), normalArray(std::move(normalArray)), 
	texCoordArray(std::move(texCoordArray)), indexArray(std::move(indexArray)),
	numVertices(numVertices), numIndices(numIndices), vao(0), vb(0), ib(0), nb(0), tb(0)
{ }


/* Build Model from generated terrain. */
Model* generate_terrain(const unsigned int world_size, const float world_xz_scale)
{
	const float tex_scale{ 1.0f / 4.0f }; // Scaling of texture coordinates

	// Build procedural terrain and smooth result
	std::vector<float> proc_terrain = diamondsquare(world_size);
	mean(proc_terrain, 5);

	const size_t vertex_count = static_cast<size_t>(world_size) * world_size;
	const size_t triangle_count = static_cast<size_t>(world_size - 1) * static_cast<size_t>(world_size - 1) * 2ull;
	// TODO: Array sizes too large here?
	// Since vertices are ordered in a cartesian grid the x and y positions might not be needed?
	// It might be possible to lower precision for the height values
	// It might be possible to use integer types for some or all of these values
	std::vector<GLfloat> vertex_array(vertex_count * 3);
	std::vector<GLfloat> normal_array(vertex_count * 3);
	std::vector<GLfloat> tex_coord_array(vertex_count * 2);
	std::vector<GLuint> index_array(triangle_count * 3);

	// Fill vertex, texture coordinate and index array
	for (unsigned int x = 0; x < world_size; x++) {
		for (unsigned int z = 0; z < world_size; z++) {
			size_t index = x + z * static_cast<size_t>(world_size);
			float y = proc_terrain[index];
			if (y < terrain_struct.min_height)
				terrain_struct.min_height = y;
			if (y > terrain_struct.max_height)
				terrain_struct.max_height = y;

			vertex_array[index * 3] = x * world_xz_scale;
			vertex_array[index * 3 + 1] = y;
			vertex_array[index * 3 + 2] = z * world_xz_scale;

			/* Scaled texture coordinates. */
			tex_coord_array[index * 2 + 0] = static_cast<float>(x) * tex_scale;
			tex_coord_array[index * 2 + 1] = static_cast<float>(z) * tex_scale;

			if ((x != world_size - 1) && (z != world_size - 1)) {
				index = (x + z * static_cast<size_t>(world_size - 1)) * 6;
				// Triangle 1
				index_array[index] = x + z * world_size;
				index_array[index + 1] = x + (z + 1) * world_size;
				index_array[index + 2] = x + 1 + z * world_size;
				// Triangle 2
				index_array[index + 3] = x + 1 + z * world_size;
				index_array[index + 4] = x + (z + 1) * world_size;
				index_array[index + 5] = x + 1 + (z + 1) * world_size;
			}
		}
	}

	// Calculate normals (cross product of two vectors along current triangle)
	const size_t offset = static_cast<size_t>(world_size) * 3;
	for (unsigned int x = 0; x < world_size; x++) {
		for (unsigned int z = 0; z < world_size; z++) {
			size_t index = (x + z * static_cast<size_t>(world_size)) * 3;
			// Initialize normals along edges to pointing straight up
			if (x == 0 || (x == world_size - 1) || z == 0 || (z == world_size - 1)) {
				normal_array[index] = 0.0;
				normal_array[index + 1] = 1.0;
				normal_array[index + 2] = 0.0;
			}
			// Inside edges, here the required indices are in bounds
			else {
				glm::vec3 p0(vertex_array[index + offset], vertex_array[index + 1 + offset], vertex_array[index + 2 + offset]);
				glm::vec3 p1(vertex_array[index - offset], vertex_array[index - offset + 1], vertex_array[index - offset + 2]);
				glm::vec3 p2(vertex_array[index - 3], vertex_array[index - 2], vertex_array[index - 1]);
				glm::vec3 a(p1 - p0);
				glm::vec3 b(p2 - p0);
				glm::vec3 normal = glm::cross(a, b);

				normal_array[index] = normal.x;
				normal_array[index + 1] = normal.y;
				normal_array[index + 2] = normal.z;
			}
		}
	}

	// Create Model and upload to GPU (formerly LoadModelData)
	Model* m = new Model(std::move(vertex_array), std::move(normal_array), 
		std::move(tex_coord_array), std::move(index_array), 
		static_cast<GLsizei>(vertex_count), static_cast<GLsizei>(triangle_count) * 3);

	glGenVertexArrays(1, &m->vao);
	glGenBuffers(1, &m->vb);
	glGenBuffers(1, &m->ib);
	glGenBuffers(1, &m->nb);
	glGenBuffers(1, &m->tb);

	// ReloadModelData() functionality below
	const GLsizeiptr vert_size = m->numVertices * sizeof(GLfloat);
	glBindVertexArray(m->vao);
	glBindBuffer(GL_ARRAY_BUFFER, m->vb);
	glBufferData(GL_ARRAY_BUFFER, vert_size * 3, m->vertexArray.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->numIndices * sizeof(GLuint), m->indexArray.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, m->nb);
	glBufferData(GL_ARRAY_BUFFER, vert_size * 3, m->normalArray.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, m->tb);
	glBufferData(GL_ARRAY_BUFFER, vert_size * 2, m->texCoordArray.data(), GL_STATIC_DRAW);

	return m;
}


/* Load chosen cubemap textures. Based on code by Joey de Vries: https://learnopengl.com/Advanced-OpenGL/Cubemaps */
void load_cubemap(std::vector<GLuint> &skybox_tex)
{
	const std::vector<std::string> skybox_paths = {
		"stormydays", "hw_morning", "sb_frozen", "ame_starfield" };
	skybox_tex.resize(skybox_paths.size());

	for (unsigned int skybox_index{}; skybox_index < skybox_paths.size(); skybox_index++)
	{
		std::string skybox_path = "tex/skybox/" + skybox_paths.at(skybox_index % skybox_paths.size());
		std::vector<std::string> faces
		{
			skybox_path + "/front.tga",
			skybox_path + "/back.tga",
			skybox_path + "/top.tga",
			skybox_path + "/bottom.tga",
			skybox_path + "/right.tga",
			skybox_path + "/left.tga"
		};

		glGenTextures(1, &skybox_tex[skybox_index]);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex[skybox_index]);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		int width, height, nr_channels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nr_channels, 0);
			if (!data)
			{
				std::cerr << "loadCubemap failed: texture " << faces[i] << " failed to load\n";
				stbi_image_free(data);
				exit(EXIT_FAILURE);
			}
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
	}
}


/* Exits the program on unrecoverable error, printing an error string to stderr */
void exit_on_error(const char* error)
{
	std::cerr << "Unrecoverable error: " << error << std::endl;
	exit(EXIT_FAILURE);
}
