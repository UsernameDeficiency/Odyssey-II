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
#include "util_misc.h" // generate_terrain, debug_message, exit_on_error, load_skybox_tex
#include "util_callback.h" // GLFW callbacks, update_physics
#include "util_shader.h"

// Initialize openGL, GLAD and GLFW
static GLFWwindow* init_gl(const bool debug_context)
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
	glClearColor(fog_color.r, fog_color.g, fog_color.b, 1.0f); // Fog color
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
		glDebugMessageCallback(debug_message, nullptr);
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
static void init_graphics(const float world_xz_scale, const float world_y_scale, const float tex_scale, Terrain_texture_ids &terrain_tex_ids)
{
	// Initialize procedural terrain. Size should be square with width 2^n for some integer n.
	terrain_shader = new Shader("shader/terrain.vert", "shader/terrain.frag");
	terrain_shader->use();

	/* Load pregenerated terrain data from terrainMap, calculate procedural terrain
	   and add proc_terrain values to the pregenerated map. */
	std::vector<float> proc_terrain = diamondsquare(world_size);
	mean(&proc_terrain, 5); // LP filter diamond-square terrain
	m_terrain = generate_terrain(proc_terrain, world_xz_scale, world_y_scale, tex_scale);

	camera.position = glm::vec3(world_size * world_xz_scale / 2, 0.0f, world_size * world_xz_scale / 2);
	// Load terrain textures and upload to texture units
	terrain_shader->load_stb_texture_ref("tex/rock_08.png", &terrain_tex_ids.snow_tex, false); // Alt. rock_03
	terrain_shader->load_stb_texture_ref("tex/mud_leaves.png", &terrain_tex_ids.grass_tex, false);
	terrain_shader->load_stb_texture_ref("tex/red_dirt_mud_01.png", &terrain_tex_ids.rock_tex, false);
	terrain_shader->load_stb_texture_ref("tex/brown_mud_rocks_01.png", &terrain_tex_ids.bottom_tex, false);
	terrain_shader->set_int("snowTex", 0);
	terrain_shader->set_int("grassTex", 1);
	terrain_shader->set_int("rockTex", 2);
	terrain_shader->set_int("bottomTex", 3);
	terrain_shader->set_bool("drawFog", false);
	terrain_shader->set_vec3("fogColor", fog_color);

	float terrain_height = terrain_struct.max_height - terrain_struct.min_height;
	terrain_struct.sea_y_pos = terrain_struct.min_height + terrain_height / 3;
	terrain_shader->set_float("minHeight", terrain_struct.min_height);
	terrain_shader->set_float("maxHeight", terrain_struct.max_height);
	terrain_shader->set_float("seaHeight", terrain_struct.sea_y_pos);
	terrain_shader->set_float("snowHeight", terrain_struct.max_height - terrain_height / 3);

	// Initialize skybox cubemap and vertices
	skybox_shader = new Shader("shader/skybox.vert", "shader/skybox.frag");
	skybox_shader->use();

	float skybox_vertices[] = {
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
	unsigned int skybox_vbo;
	glGenVertexArrays(1, &skybox_vao);
	glGenBuffers(1, &skybox_vbo);
	glBindVertexArray(skybox_vao);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

	load_skybox_tex(skybox_paths.at(0));

	skybox_shader->set_bool("draw_fog", false);
	skybox_shader->set_vec3("fogColor", fog_color);

	// Add flat water vertex, generate_terrain must run first
	water_shader = new Shader("shader/water.vert", "shader/water.frag");
	water_shader->use();

	GLfloat water_surface_vert[] = {
		// Triangle 1
		0.0f, terrain_struct.sea_y_pos, 0.0f,
		0.0f, terrain_struct.sea_y_pos, world_size * world_xz_scale,
		world_size * world_xz_scale, terrain_struct.sea_y_pos, 0.0f,
		// Triangle 2
		world_size * world_xz_scale, terrain_struct.sea_y_pos, 0.0f,
		0.0f, terrain_struct.sea_y_pos, world_size * world_xz_scale,
		world_size * world_xz_scale, terrain_struct.sea_y_pos, world_size * world_xz_scale
	};

	// Allocate and activate VAO/VBO
	unsigned int surface_vbo;
	glGenVertexArrays(1, &water_vao);
	glBindVertexArray(water_vao);
	glGenBuffers(1, &surface_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, surface_vbo);
	glBufferData(GL_ARRAY_BUFFER, 2 * 9 * sizeof(GLfloat), water_surface_vert, GL_STATIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(water_shader->id, "inPos"));
	glVertexAttribPointer(glGetAttribLocation(water_shader->id, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);
	water_shader->set_bool("draw_fog", false);
	water_shader->set_bool("extraWaves", false);
	water_shader->set_vec3("fogColor", fog_color);
	water_shader->set_float("worldSize", world_xz_scale * world_size);
}


int main()
{
	// Program settings and variables
	const float world_xz_scale = 16.0f;
	const float world_y_scale = 2.0f;
	const float tex_scale = 200.0f;
	const bool debug_context = true; // Enable/disable debugging context and prints
	float last_time = 0.0f;
	Terrain_texture_ids terrain_tex;

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
	GLFWwindow* window = init_gl(debug_context);
	init_graphics(world_xz_scale, world_y_scale, tex_scale, terrain_tex);
	glfwShowWindow(window);

	// Main render loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate frame time
		float current_time = (float)glfwGetTime();
		float delta_time = current_time - last_time;
		last_time = current_time;

		// Update physics and render screen
		update_physics(delta_time, world_xz_scale);
		
		// Clear screen and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --------- Draw skybox ---------
		glDepthMask(GL_FALSE); // Disable depth writes
		skybox_shader->use();
		glBindVertexArray(skybox_vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);

		glm::mat4 world_to_view = glm::mat4(glm::mat3(camera.get_view_matrix())); // Remove translation from the view matrix
		skybox_shader->set_mat4_f("worldToView", world_to_view);
		skybox_shader->set_mat4_f("projection", camera.projection);

		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);

		// --------- Draw terrain ---------
		terrain_shader->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, terrain_tex.snow_tex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, terrain_tex.grass_tex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, terrain_tex.rock_tex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, terrain_tex.bottom_tex);

		terrain_shader->set_mat4_f("worldToView", camera.get_view_matrix());
		terrain_shader->set_mat4_f("projection", camera.projection);

		DrawModel(m_terrain, terrain_shader->id, "inPos", "inNormal", "inTexCoord");

		// --------- Draw water surface ---------
		water_shader->use();
		glBindVertexArray(water_vao);

		water_shader->set_mat4_f("worldToView", camera.get_view_matrix());
		water_shader->set_mat4_f("projection", camera.projection);
		water_shader->set_vec3("cameraPos", camera.position);
		water_shader->set_float("time", (float)glfwGetTime());

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		print_fps(delta_time);
		glfwPollEvents();
	}

	// Render loop exited, close window and exit
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
