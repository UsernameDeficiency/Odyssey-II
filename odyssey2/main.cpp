#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "main.h"
#include "loadobj.h"
#include "Load_TGA_data.h" // loadTGATextureData
#include "diamondsquare.h"
#include "util_misc.h" // generateTerrain, debugMessage, exit_on_error
#include "util_callback.h" // GLFW callbacks, updatePhysics
#include "util_shader.h"
#include <vector>


// Initialize openGL, GLAD and GLFW
static void initGL()
{
	// --------- Initialize GLFW ---------
	if (!glfwInit())
		exit_on_error("glfwInit failed");

	// Create GLFW window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // Tesselation support
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // GL_DEBUG_OUTPUT support
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA
	glfwWindowHint(GLFW_DEPTH_BITS, 32); // Increase z buffer size for fog and z-fighting
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Increase z buffer size for fog and z-fighting
	window = glfwCreateWindow(window_w, window_h, "Odyssey II", NULL, NULL);
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

	// --------- Initialize OpenGL ---------
	glClearColor(0.7, 0.7, 0.7, 1.0f); // Fog color
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE); // Enable MSAA
	glEnable(GL_BLEND); // Enable transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// OpenGL debugging
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debugMessage, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	// Callback functions
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, fb_size_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
}


/* Initialize procedural and pregenerated terrain and add them together.
   Both terrain maps should be square with width 2^n for some integer n. */
static void initTerrain()
{
	terrainShader = new Shader("shader/terrain.vert", "shader/terrain.frag");
	terrainShader->use();

	/* Load pregenerated terrain data from terrainMap, calculate procedural terrain
	   and add procTerrain values to the pregenerated map. */
	LoadTGATextureData(terrainMap, &terrainTex); // TODO: Replace with loadStbTextureStruct
	std::cout << ".";
	//terrainShader->loadStbTextureStruct(terrainMap, &terrainTex);
	//terrainShader->loadStbTextureRef(terrainMap, &terrainTex.texID, true);
	float* procTerrain = diamondsquare(terrainTex.width, (int)terrainTex.width / (int)FILTER_DIV);
	mTerrain = generateTerrain(&terrainTex, procTerrain, world_xz_scale, world_y_scale, tex_scale);
	std::cout << ".";

	world_size = (float)terrainTex.width; // Assuming square terrain map
	const float yPos = getPosy(world_size / 2, world_size / 2, mTerrain->vertexArray, &terrainTex) + camera.height;
	camera.Position = glm::vec3(world_size * world_xz_scale / 2, yPos, world_size * world_xz_scale / 2);
	// Load terrain textures and upload to texture units
	terrainShader->loadStbTextureRef("tex/snow.png", &snowTex, false);
	terrainShader->loadStbTextureRef("tex/mud_rocks.png", &rockTex, false);
	terrainShader->loadStbTextureRef("tex/mud.png", &bottomTex, false);
	terrainShader->setInt("grassTex", 0);
	terrainShader->setInt("rockTex", 1);
	terrainShader->setInt("bottomTex", 2);
	terrainShader->setInt("draw_fog", draw_fog);
	terrainShader->setFloat("seaLevel", sea_y_pos);
}


// Initialize skybox cubemap and vertices
static void initSkybox(void)
{
	skyboxShader = new Shader("shader/skybox.vert", "shader/skybox.frag");
	skyboxShader->use();

	// Skyboxes: ely_cloudtop, miramar, stormydays
	std::string skyboxPath = "tex/skybox/stormydays/";
	std::vector<std::string> faces
	{
		skyboxPath + "front.tga",
		skyboxPath + "back.tga",
		skyboxPath + "top.tga",
		skyboxPath + "bottom.tga",
		skyboxPath + "right.tga",
		skyboxPath + "left.tga"
	};

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	skyboxTex = loadCubemap(faces);
	skyboxShader->setInt("skybox", 0);
	skyboxShader->setInt("draw_fog", draw_fog);
}


/* Add flat water vertex. generateTerrain must run before this. */
static void initWaterSurface()
{
	waterShader = new Shader("shader/water.vert", "shader/water.frag");
	waterShader->use();

	GLfloat waterSurfaceVert[] = {
		// Triangle 1
		0.0f, sea_y_pos, 0.0f,
		0.0f, sea_y_pos, world_size * world_xz_scale,
		world_size * world_xz_scale, sea_y_pos, 0.0f,
		// Triangle 2
		world_size * world_xz_scale, sea_y_pos, 0.0f,
		0.0f, sea_y_pos, world_size * world_xz_scale,
		world_size * world_xz_scale, sea_y_pos, world_size * world_xz_scale
	};

	// Allocate and activate VAO/VBO
	unsigned int surfaceVBO;
	glGenVertexArrays(1, &waterVAO);
	glBindVertexArray(waterVAO);
	glGenBuffers(1, &surfaceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, surfaceVBO);
	glBufferData(GL_ARRAY_BUFFER, 2*9*sizeof(GLfloat), waterSurfaceVert, GL_STATIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(waterShader->ID, "inPos"));
	glVertexAttribPointer(glGetAttribLocation(waterShader->ID, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	waterShader->setInt("draw_fog", draw_fog);
}


// Draw screen
static void render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear screen and depth buffer

	// --------- Draw skybox ---------
	glDepthMask(GL_FALSE); // = glDisable(GL_DEPTH_TEST)?
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
	glBindTexture(GL_TEXTURE_2D, snowTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rockTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, bottomTex);

	terrainShader->setMatrix4f("worldToView", camera.GetViewMatrix());
	terrainShader->setMatrix4f("projection", camera.projection);

	DrawModel(mTerrain, terrainShader->ID, "inPos", "inNormal", "inTexCoord");

	// --------- Draw water surface ---------
	waterShader->use();
	glBindVertexArray(waterVAO);

	waterShader->setMatrix4f("worldToView", camera.GetViewMatrix());
	waterShader->setMatrix4f("projection", camera.projection);
	waterShader->setVec3("cameraPos", camera.Position);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	// --------- Swap buffers ---------
	glfwSwapBuffers(window);
}


int main(int argc, char **argv)
{
	// Initiate OpenGL and graphics
	greet();
	initGL();
	initTerrain();
	std::cout << ".";
	initSkybox();
	initWaterSurface();
	std::cout << " finished!\n";
	glfwShowWindow(window);

	// Main render loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate frame time
		float currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// Update physics and render screen
		updatePhysics();
		render();
		printFPS();
		glfwPollEvents();
	}

	// Render loop exited, close down windows and exit.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
