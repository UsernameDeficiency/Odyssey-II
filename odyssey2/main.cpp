#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>
#include "util_misc.h"
#include "util_callback.h"
#include "util_camera.h"
#include "util_shader.h"

const unsigned int world_size = 256;
Camera camera = Camera(world_size);
Terrain_heights terrain_struct; // Used by generate_terrain to set heights for water and snow
Shader* terrain_shader;
Shader* skybox_shader;
Shader* water_shader;
GLuint skybox_tex;

struct Terrain_texture_ids
{
	GLuint snow_tex, grass_tex, rock_tex, bottom_tex;
};


// Initialize openGL, GLAD and GLFW
static GLFWwindow* init_gl(const bool debug_context)
{
	// --------- Initialize GLFW ---------
	if (!glfwInit())
		exit_on_error("glfwInit failed");

	// Create GLFW window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // GL_DEBUG_OUTPUT support
	if (debug_context)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 2); // MSAA samples
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Don't show window until loading finished
	GLFWwindow *window = glfwCreateWindow(camera.window_w, camera.window_h, "Odyssey II", NULL, NULL);
	if (!window)
		exit_on_error("GLFW window creation failed");
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // 1 = Vsync

	// --------- Initialize GLAD ---------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		exit_on_error("Failed to initialize GLAD");

	// --------- Initialize OpenGL and callbacks ---------
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


// Set up terrain, skybox and water shaders
static void init_graphics(const float world_xz_scale, Terrain_texture_ids& terrain_tex_ids, unsigned int& water_vao, unsigned int& skybox_vao)
{
	const glm::vec3 fog_color = glm::vec3(0.7, 0.7, 0.7);

	// Load terrain textures and upload to texture units
	terrain_shader = new Shader("shader/terrain.vert", "shader/terrain.frag");
	terrain_shader->use();
	terrain_shader->load_stb_texture_ref("tex/snow_02_translucent.png", &terrain_tex_ids.snow_tex, false);
	terrain_shader->load_stb_texture_ref("tex/burned_ground_01.png", &terrain_tex_ids.grass_tex, false);
	terrain_shader->load_stb_texture_ref("tex/rock_06.png", &terrain_tex_ids.rock_tex, false);
	terrain_shader->load_stb_texture_ref("tex/brown_mud_rocks_01.png", &terrain_tex_ids.bottom_tex, false);
	terrain_shader->set_int("snowTex", 0);
	terrain_shader->set_int("grassTex", 1);
	terrain_shader->set_int("rockTex", 2);
	terrain_shader->set_int("bottomTex", 3);
	terrain_shader->set_bool("drawFog", false);
	terrain_shader->set_vec3("fogColor", fog_color);

	// Set multitexturing height limits
	const float terrain_height = terrain_struct.max_height - terrain_struct.min_height;
	terrain_struct.sea_y_pos = terrain_struct.min_height + terrain_height / 3;
	terrain_shader->set_float("minHeight", terrain_struct.min_height);
	terrain_shader->set_float("maxHeight", terrain_struct.max_height);
	terrain_shader->set_float("seaHeight", terrain_struct.sea_y_pos);
	terrain_shader->set_float("snowHeight", terrain_struct.max_height - terrain_height / 3);

	// Initialize skybox cubemap and vertices
	skybox_shader = new Shader("shader/skybox.vert", "shader/skybox.frag");
	skybox_shader->use();
	skybox_shader->set_bool("drawFog", false);
	skybox_shader->set_vec3("fogColor", fog_color);
	load_cubemap(); // Load inital skybox
	skybox_shader->set_int("skyboxTex", 0);

	// Allocate and activate skybox VAO/VBO
	const GLfloat skybox_vertices[] = {
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
	unsigned int skybox_vbo;
	glGenVertexArrays(1, &skybox_vao);
	glGenBuffers(1, &skybox_vbo);
	glBindVertexArray(skybox_vao);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

	// Initialize water surface
	water_shader = new Shader("shader/water.vert", "shader/water.frag");
	water_shader->use();
	water_shader->set_bool("drawFog", false);
	water_shader->set_bool("extraWaves", false);
	water_shader->set_vec3("fogColor", fog_color);
	water_shader->set_float("worldSize", world_xz_scale * world_size);

	// Allocate and activate VAO/VBO
	const GLfloat water_surface_vert[] = {
		// Triangle 1
		0.0f, terrain_struct.sea_y_pos, 0.0f,
		0.0f, terrain_struct.sea_y_pos, world_size * world_xz_scale,
		world_size * world_xz_scale, terrain_struct.sea_y_pos, 0.0f,
		// Triangle 2
		world_size * world_xz_scale, terrain_struct.sea_y_pos, 0.0f,
		0.0f, terrain_struct.sea_y_pos, world_size * world_xz_scale,
		world_size * world_xz_scale, terrain_struct.sea_y_pos, world_size * world_xz_scale
	};
	unsigned int surface_vbo;
	glGenVertexArrays(1, &water_vao);
	glBindVertexArray(water_vao);
	glGenBuffers(1, &surface_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, surface_vbo);
	glBufferData(GL_ARRAY_BUFFER, 2 * 9 * sizeof(GLfloat), water_surface_vert, GL_STATIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(water_shader->id, "inPos"));
	glVertexAttribPointer(glGetAttribLocation(water_shader->id, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);
}


int main()
{
	// Program settings and variables
	const float world_xz_scale = 16.0f; // TODO: Move scaling parameters into terrain generation code
	const float tex_scale = 100.0f / world_size;
	const bool debug_context = false; // Enable/disable debugging context and prints
	const bool print_fps = true;
	double last_time{};
	Terrain_texture_ids terrain_tex;
	unsigned int water_vao{}, skybox_vao{};
	camera.position = glm::vec3(camera.world_size * world_xz_scale / 2, 0.0f, camera.world_size * world_xz_scale / 2);

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
	Model* m_terrain = generate_terrain(world_size, world_xz_scale, tex_scale);
	init_graphics(world_xz_scale, terrain_tex, water_vao, skybox_vao);
	// Give GLFW mouse pointer control and show window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwShowWindow(window);

	// Save shader locations for terrain
	GLint terr_vert_loc = glGetAttribLocation(terrain_shader->id, "inPos");
	GLint terr_normal_loc = glGetAttribLocation(terrain_shader->id, "inNormal");
	GLint terr_tex_loc = glGetAttribLocation(terrain_shader->id, "inTexCoord");

	// Main render loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate frame time and update physics state
		double current_time = glfwGetTime();
		double delta_time = current_time - last_time;
		last_time = current_time;
		camera.process_keyboard(m_terrain->vertexArray.data(), world_xz_scale, delta_time); // Update player state
		if (print_fps)
		{
			static unsigned int acc_frames;
			static double acc_time;
			acc_time += delta_time;
			acc_frames++;

			if (acc_time > 1.0f)
			{
				std::cout << "FPS: " << acc_frames / acc_time << "\n";
				acc_time = 0.0f;
				acc_frames = 0;
			}
		}
		
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

		glBindVertexArray(m_terrain->vao); // Select VAO
		glBindBuffer(GL_ARRAY_BUFFER, m_terrain->vb);

		glVertexAttribPointer(terr_vert_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(terr_vert_loc);

		// Normals
		glBindBuffer(GL_ARRAY_BUFFER, m_terrain->nb);
		glVertexAttribPointer(terr_normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(terr_normal_loc);

		// VBO for texture coordinate data
		glBindBuffer(GL_ARRAY_BUFFER, m_terrain->tb);
		glVertexAttribPointer(terr_tex_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(terr_tex_loc);

		glDrawElements(GL_TRIANGLES, m_terrain->numIndices, GL_UNSIGNED_INT, 0L);

		// --------- Draw water surface ---------
		water_shader->use();
		glBindVertexArray(water_vao);

		water_shader->set_mat4_f("worldToView", camera.get_view_matrix());
		water_shader->set_mat4_f("projection", camera.projection);
		water_shader->set_vec3("cameraPos", camera.position);
		water_shader->set_float("time", static_cast<float>(glfwGetTime()));

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Render loop exited, close window and exit
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
