/* Miscellaneous utility functions for the main program */
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "glm/vec3.hpp"
#include "loadobj.h" // Model*
#include "main.h"


/* Print average FPS once every second */
void print_fps(float& delta_time)
{
	static unsigned int acc_frames;
	static float acc_time;

	acc_time += delta_time;
	acc_frames++;

	if (acc_time > 1.0f)
	{
		std::cout << "FPS: " << acc_frames / acc_time << "\n";
		acc_time = 0.0f;
		acc_frames = 0;
	}
}


/* Interpolate y values over the vertex at position (x, z). No bounds checking for x and z. */
float get_pos_y(float x, float z, const GLfloat* vertex_array, const float world_xz_scale)
{
	float y_pos, y1, y2, y3;
	x /= world_xz_scale;
	z /= world_xz_scale;
	int x_tile = (int)x;
	int z_tile = (int)z;
	float x_pos = (x - x_tile);
	float z_pos = (z - z_tile);

	// Interpolate over the triangle at the current player position.
	// Interpolation is done into higher x and z, therefore x and z must be below upper array bounds
	y1 = vertex_array[(x_tile + z_tile * world_size) * 3 + 1];
	y2 = vertex_array[((x_tile + 1) + z_tile * world_size) * 3 + 1];
	y3 = vertex_array[(x_tile + (z_tile + 1) * world_size) * 3 + 1];

	y_pos = y1 + x_pos * (y2 - y1) + z_pos * (y3 - y1);

	return y_pos;
}


/* OpenGL debug callback modified for Odyssey, based on code by Joey de Vries: https://learnopengl.com */
void APIENTRY debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
	// Ignore non-significant error codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API\n"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window system\n"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader compiler\n"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third party\n"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application\n"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other\n"; break;
	}

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error\n"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated behaviour\n"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined behaviour\n"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability\n"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance\n"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker\n"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push group\n"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop group\n"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other\n"; break;
	}

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: High\n"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: Medium\n"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: Low\n"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: Notification\n"; break;
	}
	std::cout << std::endl;
}


/* Exits the program on unrecoverable error, printing an error string to stderr */
void exit_on_error(const char* error)
{
	std::cerr << "Unrecoverable error: " << error << std::endl;
	exit(EXIT_FAILURE);
}


/* Build Model from generated terrain. */
Model* generate_terrain(std::vector<float> proc_terrain, float world_xz_scale, float world_y_scale, float tex_scale)
{
	const unsigned int vertex_count = world_size * world_size;
	const unsigned int triangle_count = (world_size - 1) * (world_size - 1) * 2;
	unsigned int x, z;
	GLfloat* vertex_array = new GLfloat[sizeof(GLfloat) * 3 * vertex_count]; // TODO: Array sizes too large here?
	GLfloat* normal_array = new GLfloat[sizeof(GLfloat) * 3 * vertex_count];
	GLfloat* tex_coord_array = new GLfloat[sizeof(GLfloat) * 2 * vertex_count];
	GLuint* index_array = new GLuint[sizeof(GLuint) * triangle_count * 3];
	tex_scale /= world_size;

	// Fill vertex, texture coordinate and index array
	for (x = 0; x < world_size; x++) {
		for (z = 0; z < world_size; z++) {
			int index = x + z * world_size;

			// Add pregenerated and diamond square heights
			float y = proc_terrain[index] * world_y_scale;
			if (y < terrain_struct.min_height)
				terrain_struct.min_height = y;
			if (y > terrain_struct.max_height)
				terrain_struct.max_height = y;

			vertex_array[index * 3 + 0] = x * world_xz_scale;
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
	for (x = 0; x < world_size; x++) {
		for (z = 0; z < world_size; z++) {
			int index = (x + z * world_size) * 3;
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
	Model* model = LoadDataToModel(
		vertex_array,
		normal_array,
		tex_coord_array,
		index_array,
		vertex_count,
		triangle_count * 3);

	return model;
}


/* Load a cubemap texture. Based on code by Joey de Vries: https://learnopengl.com/Advanced-OpenGL/Cubemaps */
unsigned int load_cubemap(std::vector<std::string> faces)
{
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

	return texture_id;
}


/* Load skybox texture from the given folder. */
void load_skybox_tex(std::string skybox_path)
{
	skybox_shader->use();
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

	skybox_tex = load_cubemap(faces);
	skybox_shader->set_int("skyboxTex", 0);
}
