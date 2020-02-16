#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include <vector>
#include "main.h"
#include "loadobj.h"
#include "diamondsquare.h"
#include "terrain_filter.h"
#include "util_misc.h" // generateTerrain, debugMessage, exit_on_error, loadSkyboxTex
#include "util_callback.h" // GLFW callbacks, updatePhysics
#include "util_shader.h"

// Initialize openGL, GLAD and GLFW
static GLFWwindow* initGL(const bool debug_context)
{
	// --------- Initialize GLFW ---------
	if (!glfwInit())
		exit_on_error("glfwInit failed");

	// Create GLFW window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	if (debug_context)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // GL_DEBUG_OUTPUT support
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	}
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 2); // MSAA samples
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Don't show window until loading finished
	GLFWwindow *window = glfwCreateWindow(window_w, window_h, "Odyssey II", NULL, NULL);
	if (!window)
		exit_on_error("GLFW window creation failed");
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // 1 = Vsync
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Give window mouse pointer control
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	// --------- Initialize GLAD ---------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		exit_on_error("Failed to initialize GLAD");

	// --------- Initialize OpenGL and callbacks ---------
	glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f); // Fog color
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE); // Enable MSAA
	glEnable(GL_BLEND); // Enable transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// OpenGL debugging
	if (debug_context)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debugMessage, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glfwSetErrorCallback(error_callback);
	}

	// Callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, fb_size_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);

	return window;
}


// Set up terrain, skybox and water
static void initGraphics(const float world_xz_scale, const float world_y_scale, const float tex_scale, terrainTextureIDs &terrainTexIDs)
{
	// Initialize procedural terrain. Size should be square with width 2^n for some integer n.
	terrainShader = new Shader("shader/terrain.vert", "shader/terrain.frag");
	terrainShader->use();

	/* Load pregenerated terrain data from terrainMap, calculate procedural terrain
	   and add procTerrain values to the pregenerated map. */
	std::vector<float> procTerrain = diamondsquare(world_size);
	mean(&procTerrain, 5); // LP filter diamond-square terrain
	mTerrain = generateTerrain(procTerrain, world_xz_scale, world_y_scale, tex_scale);

	camera.Position = glm::vec3(world_size * world_xz_scale / 2, 0.0f, world_size * world_xz_scale / 2);
	// Load terrain textures and upload to texture units
	terrainShader->loadStbTextureRef("tex/rock_08.png", &terrainTexIDs.snowTex, false); // Alt. rock_03
	terrainShader->loadStbTextureRef("tex/mud_leaves.png", &terrainTexIDs.grassTex, false);
	terrainShader->loadStbTextureRef("tex/red_dirt_mud_01.png", &terrainTexIDs.rockTex, false);
	terrainShader->loadStbTextureRef("tex/brown_mud_rocks_01.png", &terrainTexIDs.bottomTex, false);
	terrainShader->setInt("snowTex", 0);
	terrainShader->setInt("grassTex", 1);
	terrainShader->setInt("rockTex", 2);
	terrainShader->setInt("bottomTex", 3);
	terrainShader->setBool("drawFog", false);
	terrainShader->setVec3("fogColor", fogColor);

	float terrainHeight = terrainStruct.maxHeight - terrainStruct.minHeight;
	terrainStruct.sea_y_pos = terrainStruct.minHeight + terrainHeight / 3;
	terrainShader->setFloat("minHeight", terrainStruct.minHeight);
	terrainShader->setFloat("maxHeight", terrainStruct.maxHeight);
	terrainShader->setFloat("seaHeight", terrainStruct.sea_y_pos);
	terrainShader->setFloat("snowHeight", terrainStruct.maxHeight - terrainHeight / 3);

	// Initialize skybox cubemap and vertices
	skyboxShader = new Shader("shader/skybox.vert", "shader/skybox.frag");
	skyboxShader->use();

	float skyboxVertices[] = {
		-5.0f,  5.0f, -5.0f, -5.0f, -5.0f, -5.0f, 5.0f, -5.0f, -5.0f,
		 5.0f, -5.0f, -5.0f,  5.0f,  5.0f, -5.0f, -5.0f,  5.0f, -5.0f,

		-5.0f, -5.0f,  5.0f, -5.0f, -5.0f, -5.0f, -5.0f,  5.0f, -5.0f,
		-5.0f,  5.0f, -5.0f, -5.0f,  5.0f,  5.0f, -5.0f, -5.0f,  5.0f,

		 5.0f, -5.0f, -5.0f, 5.0f, -5.0f,  5.0f, 5.0f,  5.0f,  5.0f,
		 5.0f,  5.0f,  5.0f, 5.0f,  5.0f, -5.0f, 5.0f, -5.0f, -5.0f,

		-5.0f, -5.0f,  5.0f, -5.0f,  5.0f,  5.0f, 5.0f,  5.0f,  5.0f,
		 5.0f,  5.0f,  5.0f, 5.0f, -5.0f,  5.0f, -5.0f, -5.0f,  5.0f,

		-5.0f,  5.0f, -5.0f, 5.0f,  5.0f, -5.0f, 5.0f,  5.0f,  5.0f,
		 5.0f,  5.0f,  5.0f, -5.0f,  5.0f,  5.0f, -5.0f,  5.0f, -5.0f,

		-5.0f, -5.0f, -5.0f, -5.0f, -5.0f,  5.0f, 5.0f, -5.0f, -5.0f,
		 5.0f, -5.0f, -5.0f, -5.0f, -5.0f,  5.0f, 5.0f, -5.0f,  5.0f
	};

	// Allocate and activate VAO/VBO
	unsigned int skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

	loadSkyboxTex(skyboxPaths.at(0));

	skyboxShader->setBool("draw_fog", false);
	skyboxShader->setVec3("fogColor", fogColor);

	// Add flat water vertex, generateTerrain must run first
	waterShader = new Shader("shader/water.vert", "shader/water.frag");
	waterShader->use();

	GLfloat waterSurfaceVert[] = {
		// Triangle 1
		0.0f, terrainStruct.sea_y_pos, 0.0f,
		0.0f, terrainStruct.sea_y_pos, world_size * world_xz_scale,
		world_size * world_xz_scale, terrainStruct.sea_y_pos, 0.0f,
		// Triangle 2
		world_size * world_xz_scale, terrainStruct.sea_y_pos, 0.0f,
		0.0f, terrainStruct.sea_y_pos, world_size * world_xz_scale,
		world_size * world_xz_scale, terrainStruct.sea_y_pos, world_size * world_xz_scale
	};

	// Allocate and activate VAO/VBO
	unsigned int surfaceVBO;
	glGenVertexArrays(1, &waterVAO);
	glBindVertexArray(waterVAO);
	glGenBuffers(1, &surfaceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, surfaceVBO);
	glBufferData(GL_ARRAY_BUFFER, 2 * 9 * sizeof(GLfloat), waterSurfaceVert, GL_STATIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(waterShader->ID, "inPos"));
	glVertexAttribPointer(glGetAttribLocation(waterShader->ID, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	waterShader->setBool("draw_fog", false);
	waterShader->setBool("extraWaves", false);
	waterShader->setVec3("fogColor", fogColor);
	waterShader->setFloat("worldSize", world_xz_scale * world_size);
}


int main(int argc, char **argv)
{
	// Program settings and variables
	const float world_xz_scale = 16.0f;
	const float world_y_scale = 2.0f;
	const float tex_scale = 200.0f;
	const bool debug_context = true; // Enable/disable debugging context and prints
	float lastTime = 0.0f;
	terrainTextureIDs terrainTex;

	// Print greeting
	std::cout <<
		"------------------------------------\n"
		"       Welcome to Odyssey II!\n"
		"------------------------------------\n"
		"Move: W/A/S/D/Q/E\n"
		"Run: Shift\n"
		"Zoom: Ctrl\n"
		"Crouch: C\n"
		"Toggle flying/walking: F\n"
		"Toggle fog: F1\n"
		"Toggle water wave effect: F2\n"
		"Toggle skybox: F3\n"
		"------------------------------------\n";

	// Initiate OpenGL and graphics
	GLFWwindow* window = initGL(debug_context);
	initGraphics(world_xz_scale, world_y_scale, tex_scale, terrainTex);
	glfwShowWindow(window);

	// Main render loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate frame time
		float currentTime = (float)glfwGetTime();
		float deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Update physics and render screen
		updatePhysics(deltaTime, world_xz_scale);
		
		// Clear screen and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --------- Draw skybox ---------
		glDepthMask(GL_FALSE); // Disable depth writes
		skyboxShader->use();
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

		glm::mat4 worldToView = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Remove translation from the view matrix
		skyboxShader->setMatrix4f("worldToView", worldToView);
		skyboxShader->setMatrix4f("projection", camera.projection);

		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);

		// --------- Draw terrain ---------
		terrainShader->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, terrainTex.snowTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, terrainTex.grassTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, terrainTex.rockTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, terrainTex.bottomTex);

		terrainShader->setMatrix4f("worldToView", camera.GetViewMatrix());
		terrainShader->setMatrix4f("projection", camera.projection);

		DrawModel(mTerrain, terrainShader->ID, "inPos", "inNormal", "inTexCoord");

		// --------- Draw water surface ---------
		waterShader->use();
		glBindVertexArray(waterVAO);

		waterShader->setMatrix4f("worldToView", camera.GetViewMatrix());
		waterShader->setMatrix4f("projection", camera.projection);
		waterShader->setVec3("cameraPos", camera.Position);
		waterShader->setFloat("time", (float)glfwGetTime());

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		printFPS(deltaTime);
		glfwPollEvents();
	}

	// Render loop exited, close window and exit
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
