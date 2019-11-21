#pragma once
#include "loadobj.h" // Model
#include "Load_TGA_data.h" // TextureData
#include "util_camera.h" // Camera and camera settings
#include "util_shader.h" // Shader


/* Program settings */
/* Terrain maps: fft-terrain (256), 512_flat, 512, 2048_flat, 16384_flat 
	For loadTGA, the image needs bottom-left origin, with or without RLE */
const char terrainMap[] = "heightmap/2048_flat.tga";
// Scaling values for terrain vertices/texture coordinates
const float world_xz_scale = 8.0f;
const float world_y_scale = 5.0f;
const float tex_scale = 256.0f;


/* Global variables */
float world_size; // width = height of world map
float sea_y_pos; // Set by generateTerrain using mean height of terrain
bool drawFog = false; // Shader fog effect
glm::vec3 fogColor = glm::vec3(0.7, 0.7, 0.7);
const bool DEBUG_CONTEXT = false; // Enable/disable debugging context and prints
GLFWwindow* window = NULL;
Camera camera = Camera(); // position initialized in initTerrain
Model* mTerrain;
Shader *terrainShader, *skyboxShader, *waterShader, *fogShader;
GLuint snowTex, rockTex, bottomTex, skyboxTex;
TextureData terrainTex; // Terrain height map
unsigned int waterVAO, skyboxVAO, fogVAO;
float deltaTime = 0.0f;	// frame time for last frame
float lastTime = 0.0f;
float accTime; // used for printFPS()
int accFrames; // used for printFPS()
float mouseLastX = window_w / 2.0f;
float mouseLastY = window_h / 2.0f;
