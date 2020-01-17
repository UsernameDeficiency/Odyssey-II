#pragma once
#include <vector>
#include <string>
#include "loadobj.h" // Model
//#include "Load_TGA_data.h" // TextureData
#include "util_camera.h" // Camera and camera settings
#include "util_shader.h" // Shader


/* Program settings */
/* Terrain maps: fft-terrain (256), 512, 2048_flat
	For loadTGA, the image needs bottom-left origin, with or without RLE */
//const char terrainMap[] = "heightmap/1024.tga";
// Scaling values for terrain vertices/texture coordinates
const float world_xz_scale = 8.0f;
const float world_y_scale = 0.55f;
const float tex_scale = 128.0f;
int world_size = 1024; // TODO: width = height of world map
//TextureData terrainTex; // Terrain height map


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
