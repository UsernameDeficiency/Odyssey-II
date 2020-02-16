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
const glm::vec3 fogColor = glm::vec3(0.7, 0.7, 0.7);
const std::vector<std::string> skyboxPaths = {
	"stormydays", "hw_morning", "sb_frozen", "ame_starfield" };


/* Global variables */
struct
{
	float minHeight{ FLT_MAX };
	float maxHeight{ -FLT_MAX };
	float sea_y_pos{};
} terrainStruct; // Used by generateTerrain to set heights for water and snow
Camera camera = Camera();
Model* mTerrain;
Shader *terrainShader, *skyboxShader, *waterShader;
struct terrainTextureIDs
{
	GLuint snowTex, grassTex, rockTex, bottomTex;
};
GLuint skyboxTex;
unsigned int waterVAO, skyboxVAO;
