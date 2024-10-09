/* Miscellaneous utility functions for the main program */
#pragma once
#include <vector>
#include <cfloat>
#include <glad/glad.h>

// TODO: Potentially move Terrain_heights to terrain.h
struct Terrain_heights
{
	float min_height{ FLT_MAX };
	float max_height{ -FLT_MAX };
	float sea_y_pos{};
};

/* Load chosen cubemap textures */
void load_cubemap(std::vector<GLuint>& skybox_tex);

/* Exits the program on unrecoverable error, printing an error string to stderr */
void exit_on_error(const char* error);
