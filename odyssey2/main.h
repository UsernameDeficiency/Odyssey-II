#pragma once
#include <vector>
#include <string>
#include <float.h>
#include "loadobj.h" // Model
#include "util_camera.h" // Camera and camera settings
#include "util_shader.h" // Shader


/* Program settings */
// Scaling values for terrain vertices/texture coordinates
const unsigned int world_size = 2048; // width = height of world map
const glm::vec3 fog_color = glm::vec3(0.7, 0.7, 0.7);
const std::vector<std::string> skybox_paths = {
	"stormydays", "hw_morning", "sb_frozen", "ame_starfield" };


/* Global variables */
struct Terrain_heights
{
	float min_height{ FLT_MAX };
	float max_height{ -FLT_MAX };
	float sea_y_pos{};
} terrain_struct; // Used by generate_terrain to set heights for water and snow
Camera camera = Camera();
Model* m_terrain;
Shader *terrain_shader, *skybox_shader, *water_shader;
struct Terrain_texture_ids
{
	GLuint snow_tex, grass_tex, rock_tex, bottom_tex;
};
GLuint skybox_tex;
unsigned int water_vao, skybox_vao;
