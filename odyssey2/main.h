#pragma once
#include <vector>
#include <string>
#include <float.h>
#include "loadobj.h" // Model
#include "util_camera.h" // Camera and camera settings
#include "util_shader.h" // Shader


/* Program settings */
const bool DEBUG_CONTEXT = false; // Enable/disable debugging context and prints
// Scaling values for terrain vertices/texture coordinates
const float world_xz_scale = 16.0f;
const float world_y_scale = 2.0f;
const float tex_scale = 128.0f;
const unsigned int world_size = 2048; // width = height of world map
const glm::vec3 fogColor = glm::vec3(0.7, 0.7, 0.7);
const std::vector<std::string> skyboxPaths = {
	"stormydays", "hw_morning", "sb_frozen", "ame_starfield" };


/* Global variables */
// Used by generateTerrain to set heights for water and snow
struct
{
	float minHeight{ FLT_MAX };
	float maxHeight{ -FLT_MAX }; 
	float sea_y_pos{};
} terrainStruct;
GLFWwindow* window = nullptr;
Camera camera = Camera();
Model* mTerrain;
Shader *terrainShader, *skyboxShader, *waterShader;
GLuint snowTex, grassTex, rockTex, bottomTex, skyboxTex;
unsigned int waterVAO, skyboxVAO;
