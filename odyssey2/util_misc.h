/* Miscellaneous utility functions for the main program */
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "glm/vec3.hpp"
#include "loadobj.h" // Model*
#include "main.h"


/* Print average FPS once every second */
void print_fps(double& delta_time)
{
	static unsigned int acc_frames;
	static double acc_time;

	acc_time += delta_time;
	acc_frames++;

	if (acc_time > 1.0f)
	{
		std::cout << "FPS: " << acc_frames / acc_time << "\n";
		acc_time = 0.0f;
		acc_frames = 0;
	}
}


/* Build Model from generated terrain. */
Model* generate_terrain(std::vector<float> proc_terrain, const float world_xz_scale, const float world_y_scale, float tex_scale)
{
	const unsigned int vertex_count = world_size * world_size;
	const unsigned int triangle_count = (world_size - 1) * (world_size - 1) * 2;
	GLfloat* vertex_array = new GLfloat[sizeof(GLfloat) * 3 * vertex_count]; // TODO: Array sizes too large here?
	GLfloat* normal_array = new GLfloat[sizeof(GLfloat) * 3 * vertex_count];
	GLfloat* tex_coord_array = new GLfloat[sizeof(GLfloat) * 2 * vertex_count];
	GLuint* index_array = new GLuint[sizeof(GLuint) * 3 * triangle_count];
	tex_scale /= world_size;

	// Fill vertex, texture coordinate and index array
	for (unsigned int x = 0; x < world_size; x++) {
		for (unsigned int z = 0; z < world_size; z++) {
			unsigned int index = x + z * world_size;
			float y = proc_terrain[index] * world_y_scale;
			if (y < terrain_struct.min_height)
				terrain_struct.min_height = y;
			if (y > terrain_struct.max_height)
				terrain_struct.max_height = y;

			vertex_array[index * 3] = x * world_xz_scale;
			vertex_array[index * 3 + 1] = y;
			vertex_array[index * 3 + 2] = z * world_xz_scale;

			/* Scaled texture coordinates. */
			tex_coord_array[index * 2 + 0] = (float)x * tex_scale;
			tex_coord_array[index * 2 + 1] = (float)z * tex_scale;

			if ((x != world_size - 1) && (z != world_size - 1)) {
				index = (x + z * (world_size - 1)) * 6;
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
	for (unsigned int x = 0; x < world_size; x++) {
		for (unsigned int z = 0; z < world_size; z++) {
			unsigned int index = (x + z * world_size) * 3;
			// Initialize normals along edges to pointing straight up
			if (x == 0 || (x == world_size - 1) || z == 0 || (z == world_size - 1)) {
				normal_array[index] = 0.0;
				normal_array[index + 1] = 1.0;
				normal_array[index + 2] = 0.0;
			}
			// Inside edges, here the required indices are in bounds
			else {
				glm::vec3 p0(vertex_array[index + world_size * 3], vertex_array[index + 1 + world_size * 3], vertex_array[index + 2 + world_size * 3]);
				glm::vec3 p1(vertex_array[index - world_size * 3], vertex_array[index - world_size * 3 + 1], vertex_array[index - world_size * 3 + 2]);
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

	// Create Model and upload to GPU
	Model* model = LoadDataToModel(vertex_array, normal_array, tex_coord_array,
		index_array, vertex_count, triangle_count * 3);
	return model;
}


/* Load a cubemap texture. Based on code by Joey de Vries: https://learnopengl.com/Advanced-OpenGL/Cubemaps */
void load_cubemap(std::string skybox_path)
{
	skybox_path = "tex/skybox/" + skybox_path;
	std::vector<std::string> faces
	{
		skybox_path + "/front.tga",
		skybox_path + "/back.tga",
		skybox_path + "/top.tga",
		skybox_path + "/bottom.tga",
		skybox_path + "/right.tga",
		skybox_path + "/left.tga"
	};

	// Make sure the wanted texture has not already been loaded. This method makes sure new memory is not allocated
	// every time the skybox is changed but still loads the texture from disk every time.
	unsigned int texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
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

	skybox_tex = texture_id;
}


/* Exits the program on unrecoverable error, printing an error string to stderr */
void exit_on_error(const char* error)
{
	std::cerr << "Unrecoverable error: " << error << std::endl;
	exit(EXIT_FAILURE);
}
