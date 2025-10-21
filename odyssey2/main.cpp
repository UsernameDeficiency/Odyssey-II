#include "callback.h"
#include "camera.h"
#include "io.h"
#include "shader.h"
#include "terrain.h"
#include "util_misc.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <iostream>

struct Terrain_texture_ids
{
	GLuint snow_tex, grass_tex, rock_tex, bottom_tex;
};

// Initialize openGL, GLAD and GLFW
// TODO: Move debug_context, MSAA samples and maybe more to settings.ini
static GLFWwindow* init_gl()
{
	// Enable/disable debugging context and prints
	constexpr bool debug_context{ false };

	// --------- Initialize GLFW ---------
	if (!glfwInit())
		exit_on_error("glfwInit failed");

	// Create GLFW window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	if constexpr (debug_context)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // GL_DEBUG_OUTPUT support
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwSetErrorCallback(error_callback);
	}
	else
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0); // no GL_DEBUG_OUTPUT support needed
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
	}
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 2); // MSAA samples
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Don't show window until loading finished

	unsigned requested_window_w{ read_value_from_ini("window_w", 1280u) };
	unsigned requested_window_h{ read_value_from_ini("window_h", 720u) };
	GLFWwindow* window = glfwCreateWindow(requested_window_w, requested_window_h, "Odyssey II", NULL, NULL);
	if (!window)
		exit_on_error("GLFW window creation failed");
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // 1 = Vsync

	// --------- Initialize GLAD ---------
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
		exit_on_error("Failed to initialize GLAD");

	// --------- Initialize OpenGL and callbacks ---------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE); // Enable MSAA
	glEnable(GL_BLEND); // Enable transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// OpenGL debugging
	if constexpr (debug_context)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debug_message, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	// Callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, fb_size_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);

	return window;
}

// Set up terrain, skybox and water shaders
// TODO: Move fog_color and textures to settings.ini
static void init_graphics(Terrain_texture_ids& terrain_tex_ids,
	std::vector<GLuint>& skybox_textures, Shader*& skybox_shader, Shader*& terrain_shader, Shader*& water_shader, const Terrain& terrain)
{
	const glm::vec3 fog_color{ glm::vec3(0.7, 0.7, 0.7) };

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
	const float terrain_height{ terrain.max_height - terrain.min_height };
	terrain_shader->set_float("minHeight", terrain.min_height);
	terrain_shader->set_float("maxHeight", terrain.max_height);
	terrain_shader->set_float("seaHeight", terrain.sea_height);
	terrain_shader->set_float("snowHeight", terrain.max_height - terrain_height / 3);

	// Initialize skybox cubemap and vertices
	skybox_shader = new Shader("shader/skybox.vert", "shader/skybox.frag");
	skybox_shader->use();
	skybox_shader->set_bool("drawFog", false);
	skybox_shader->set_vec3("fogColor", fog_color);
	load_cubemap(skybox_textures); // Load initial skybox
	skybox_shader->set_int("skyboxTex", 0);

	// Allocate and activate skybox VBO
	const GLfloat skybox_vertices[]{
		-5.0f, 5.0f, -5.0f, -5.0f, -5.0f, -5.0f, 5.0f, -5.0f, -5.0f,
		5.0f, -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, -5.0f, 5.0f, -5.0f,

		-5.0f, -5.0f, 5.0f, -5.0f, -5.0f, -5.0f, -5.0f, 5.0f, -5.0f,
		-5.0f, 5.0f, -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, -5.0f, 5.0f,

		5.0f, -5.0f, -5.0f, 5.0f, -5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
		5.0f, 5.0f, 5.0f, 5.0f, 5.0f, -5.0f, 5.0f, -5.0f, -5.0f,

		-5.0f, -5.0f, 5.0f, -5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
		5.0f, 5.0f, 5.0f, 5.0f, -5.0f, 5.0f, -5.0f, -5.0f, 5.0f,

		-5.0f, 5.0f, -5.0f, 5.0f, 5.0f, -5.0f, 5.0f, 5.0f, 5.0f,
		5.0f, 5.0f, 5.0f, -5.0f, 5.0f, 5.0f, -5.0f, 5.0f, -5.0f,

		-5.0f, -5.0f, -5.0f, -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, -5.0f,
		5.0f, -5.0f, -5.0f, -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, 5.0f
	};
	unsigned int skybox_vbo;
	glGenBuffers(1, &skybox_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

	// Initialize water surface
	const float world_total_size = terrain.world_xz_scale * (terrain.world_size - 1);
	water_shader = new Shader("shader/water.vert", "shader/water.frag");
	water_shader->use();
	water_shader->set_bool("drawFog", false);
	water_shader->set_bool("extraWaves", false);
	water_shader->set_vec3("fogColor", fog_color);
	water_shader->set_float("worldSize", world_total_size);

	// Allocate and activate VAO/VBO
	const GLfloat water_surface_vert[]{
		// Triangle 1
		0.0f, terrain.sea_height, 0.0f,
		0.0f, terrain.sea_height, world_total_size,
		world_total_size, terrain.sea_height, 0.0f,
		// Triangle 2
		world_total_size, terrain.sea_height, 0.0f,
		0.0f, terrain.sea_height, world_total_size,
		world_total_size, terrain.sea_height, world_total_size
	};
	unsigned int water_vbo;
	glGenBuffers(1, &water_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, water_vbo);
	glBufferData(GL_ARRAY_BUFFER, 2ull * 9ull * sizeof(GLfloat), water_surface_vert, GL_STATIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(water_shader->id, "inPos"));
	glVertexAttribPointer(glGetAttribLocation(water_shader->id, "inPos"), 3, GL_FLOAT, GL_FALSE, 0, 0);
}

int main()
{
	std::cout << read_value_from_ini<std::string>("greeting", "");

	// Initiate OpenGL and graphics
	GLFWwindow* window{ init_gl() };

	// Generate terrain
	const unsigned int world_size{ read_value_from_ini("world_size", 128u) };
	const float world_xz_scale{ read_value_from_ini("world_xz_scale", 32.0f) };
	const Terrain terrain{ world_size, world_xz_scale };

	Shader *skybox_shader, *terrain_shader, *water_shader;
	Terrain_texture_ids terrain_tex{};
	std::vector<GLuint> skybox_textures;
	init_graphics(terrain_tex, skybox_textures, skybox_shader, terrain_shader, water_shader, terrain);

	// Initialize player camera
	Camera camera(terrain.sea_height);
	glfwSetWindowUserPointer(window, &camera); // Give callbacks access to camera
	int window_w{}, window_h{};
	glfwGetWindowSize(window, &window_w, &window_h);
	camera.aspect_ratio = static_cast<float>(window_w) / window_h; // Can be set after window initialization
	const float initial_xz_pos = terrain.world_size * terrain.world_xz_scale / 2.0f;
	camera.position = glm::vec3(initial_xz_pos, terrain.max_height, initial_xz_pos);

	// Give GLFW mouse pointer control and show window
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwShowWindow(window);

	// Save shader locations for terrain
	GLint terr_vert_loc{ glGetAttribLocation(terrain_shader->id, "inPos") };
	GLint terr_normal_loc{ glGetAttribLocation(terrain_shader->id, "inNormal") };
	GLint terr_tex_loc{ glGetAttribLocation(terrain_shader->id, "inTexCoord") };

	// Main render loop
	double last_time{};
	unsigned int skybox_index{};
	while (!glfwWindowShouldClose(window))
	{
		// Calculate frame time and update physics state
		double current_time{ glfwGetTime() };
		double delta_time{ current_time - last_time };
		last_time = current_time;
		camera.process_keyboard(terrain.terrain_model->vertexArray, terrain.world_xz_scale, delta_time); // Update player state
		// Toggle fog
		if (camera.key_state[GLFW_KEY_F1] == GLFW_PRESS)
		{
			static bool draw_fog = false;

			draw_fog = !draw_fog;
			terrain_shader->use();
			terrain_shader->set_bool("drawFog", draw_fog);
			skybox_shader->use();
			skybox_shader->set_bool("drawFog", draw_fog);
			water_shader->use();
			water_shader->set_bool("drawFog", draw_fog);
			camera.key_state[GLFW_KEY_F1] = GLFW_REPEAT;
		}
		// Toggle wave amount
		if (camera.key_state[GLFW_KEY_F2] == GLFW_PRESS)
		{
			static bool extra_waves = false;

			extra_waves = !extra_waves;
			water_shader->use();
			// TODO: Would it be more performant to switch shaders here based on
			// extra_waves instead of checking a boolean inside the shader?
			water_shader->set_bool("extraWaves", extra_waves);
			camera.key_state[GLFW_KEY_F2] = GLFW_REPEAT;
		}

		// Print FPS once every second
		if constexpr (constexpr bool print_fps{ false }; print_fps)
		{
			static unsigned int acc_frames{};
			static double acc_time{};
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
		glBindVertexArray(skybox_shader->vao);
		glActiveTexture(GL_TEXTURE0);
		// Change skybox texture
		if (camera.key_state[GLFW_KEY_F3] == GLFW_PRESS)
		{
			skybox_index = ++skybox_index % skybox_textures.size();
			camera.key_state[GLFW_KEY_F3] = GLFW_REPEAT;
		}
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_textures[skybox_index]);

		glm::mat4 world_to_view{ glm::mat4(glm::mat3(camera.get_view_matrix())) }; // Remove translation from the view matrix
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

		glBindVertexArray(terrain.terrain_model->vao);
		glBindBuffer(GL_ARRAY_BUFFER, terrain.terrain_model->vb);

		glVertexAttribPointer(terr_vert_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(terr_vert_loc);

		// Normals
		glBindBuffer(GL_ARRAY_BUFFER, terrain.terrain_model->nb);
		glVertexAttribPointer(terr_normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(terr_normal_loc);

		// VBO for texture coordinate data
		glBindBuffer(GL_ARRAY_BUFFER, terrain.terrain_model->tb);
		glVertexAttribPointer(terr_tex_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(terr_tex_loc);

		glDrawElements(GL_TRIANGLES, terrain.terrain_model->numIndices, GL_UNSIGNED_INT, 0L);

		// --------- Draw water surface ---------
		water_shader->use();
		glBindVertexArray(water_shader->vao);

		water_shader->set_mat4_f("worldToView", camera.get_view_matrix());
		water_shader->set_mat4_f("projection", camera.projection);
		water_shader->set_vec3("cameraPos", camera.position);
		water_shader->set_float("time", static_cast<float>(glfwGetTime()));

		glDrawArrays(GL_TRIANGLES, 0, 6); // API_ID_RECOMPILE_FRAGMENT_SHADER on (only) first call

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Render loop exited, close window and exit
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
