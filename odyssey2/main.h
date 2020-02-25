#pragma once
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <float.h>

struct Terrain_heights
{
	float min_height{ FLT_MAX };
	float max_height{ -FLT_MAX };
	float sea_y_pos{};
};

class Camera;
class Shader;

/* Global variables */
extern const unsigned int world_size; // width = height of world map
extern Terrain_heights terrain_struct; // Used by generate_terrain to set heights for water and snow
extern Shader* terrain_shader, *skybox_shader, *water_shader;
extern GLuint skybox_tex;
extern Camera camera;
