#pragma once
#include <vector>
#include <string>
#include "loadobj.h" // Model
#include "util_camera.h" // Camera and camera settings
#include "util_shader.h" // Shader


/* Program settings */
// Scaling values for terrain vertices/texture coordinates
const float world_xz_scale = 16.0f;
const float world_y_scale = 2.0f;
const float tex_scale = 128.0f;
int world_size = 2048; // width = height of world map


/* Global variables */
// Used by generateTerrain to set heights for water and snow
float minHeight{ FLT_MAX }, maxHeight{ -FLT_MAX }, sea_y_pos, snow_y_pos; 
bool drawFog, extraWaves; // Shader effects
glm::vec3 fogColor = glm::vec3(0.7, 0.7, 0.7);
const bool DEBUG_CONTEXT = false; // Enable/disable debugging context and prints
GLFWwindow* window = NULL;
Camera camera = Camera(); // position initialized in initTerrain
Model* mTerrain;
Shader *terrainShader, *skyboxShader, *waterShader, *fogShader;
GLuint snowTex, grassTex, rockTex, bottomTex, skyboxTex;
unsigned int waterVAO, skyboxVAO, fogVAO;
unsigned int skyboxIndex;
std::vector<std::string> skyboxPaths = {
	"stormydays", "hw_morning", "sb_frozen", "ame_starfield" };
std::vector<int> skyboxTextureID(skyboxPaths.size(), -1);
float deltaTime = 0.0f;	// frame time for last frame
float lastTime = 0.0f;
float accTime; // for printFPS()
int accFrames; // for printFPS()
float mouseLastX = 0.0f;
float mouseLastY = window_h / 2.0f;
