/* Callback definitions for GLFW and related functions */
#pragma once
#include "main.h" // Constants and globals


// Keyboard state for key_callback/update_physics
struct
{
	enum { KEY_FORWARD, KEY_BACKWARD, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN };
} key_enum;
bool key_state[key_enum.KEY_DOWN + 1] = { false };


// Interpolate y values over the vertex at position (x, z). No bounds checking for x and z.
float get_pos_y(float x, float z, const GLfloat* vertex_array, const float world_xz_scale)
{
	x /= world_xz_scale;
	z /= world_xz_scale;
	int x_tile = (int)x;
	int z_tile = (int)z;
	float x_pos = (x - x_tile);
	float z_pos = (z - z_tile);

	// Interpolate over the triangle at the current player position.
	// Interpolation is done into higher x and z, therefore x and z must be below upper array bounds
	const float y1 = vertex_array[(x_tile + z_tile * world_size) * 3 + 1];
	const float y2 = vertex_array[((x_tile + 1) + z_tile * world_size) * 3 + 1];
	const float y3 = vertex_array[(x_tile + (z_tile + 1) * world_size) * 3 + 1];
	const float y_pos = y1 + x_pos * (y2 - y1) + z_pos * (y3 - y1);

	return y_pos;
}


// Reads keyboard state set by key_callback and updates player movement
void update_physics(const float delta_time, const float world_xz_scale)
{
	if (key_state[key_enum.KEY_FORWARD])
		camera.process_keyboard(Camera::movement::CAM_FORWARD, delta_time);
	if (key_state[key_enum.KEY_BACKWARD])
		camera.process_keyboard(Camera::movement::CAM_BACKWARD, delta_time);
	if (key_state[key_enum.KEY_LEFT])
		camera.process_keyboard(Camera::movement::CAM_LEFT, delta_time);
	if (key_state[key_enum.KEY_RIGHT])
		camera.process_keyboard(Camera::movement::CAM_RIGHT, delta_time);

	if (camera.flying)
	{
		if (key_state[key_enum.KEY_UP])
			camera.process_keyboard(Camera::movement::CAM_UP, delta_time);
		if (key_state[key_enum.KEY_DOWN])
			camera.process_keyboard(Camera::movement::CAM_DOWN, delta_time);
	}
	else
	{
		// Move player to terrain height, world_xz_scale padding ensures arrays stay in bound during interpolation
		const float world_limit = world_size * world_xz_scale - world_xz_scale;
		if (camera.position.x < 0.0f)
			camera.position.x = 0.0f;
		else if (camera.position.x > world_limit)
			camera.position.x = world_limit;
		if (camera.position.z < 0.0f)
			camera.position.z = 0.0f;
		else if (camera.position.z > world_limit)
			camera.position.z = world_limit;

		const float new_y_pos = get_pos_y(camera.position.x, camera.position.z, m_terrain->vertexArray, world_xz_scale) + camera.height;
		const float swim_height = terrain_struct.sea_y_pos + camera.height / 3;

		// Make sure player does not drown.
		if (new_y_pos > swim_height)
			camera.position.y = new_y_pos;
		else
			camera.position.y = swim_height;
	}
}


// Read keyboard input, do bounds checking for player and moving objects
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W)
		key_state[key_enum.KEY_FORWARD] = (action == GLFW_PRESS || action == GLFW_REPEAT);
	else if (key == GLFW_KEY_S)
		key_state[key_enum.KEY_BACKWARD] = (action == GLFW_PRESS || action == GLFW_REPEAT);
	else if (key == GLFW_KEY_A)
		key_state[key_enum.KEY_LEFT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
	else if (key == GLFW_KEY_D)
		key_state[key_enum.KEY_RIGHT] = (action == GLFW_PRESS || action == GLFW_REPEAT);
	else if (key == GLFW_KEY_E)
		key_state[key_enum.KEY_UP] = (action == GLFW_PRESS || action == GLFW_REPEAT);
	else if (key == GLFW_KEY_Q)
		key_state[key_enum.KEY_DOWN] = (action == GLFW_PRESS || action == GLFW_REPEAT);

	// Move faster
	else if (key == GLFW_KEY_LEFT_SHIFT)
	{
		if (action == GLFW_PRESS)
			camera.movement_speed = camera.cam_speed * 8.0f;
		else if (action == GLFW_RELEASE)
			camera.movement_speed = camera.cam_speed;
	}

	// Zoom by lowering fov
	else if (key == GLFW_KEY_LEFT_CONTROL)
	{
		if (action == GLFW_PRESS)
		{
			camera.projection = glm::perspective(glm::radians(camera.fov / 4), (GLfloat)camera.window_w / (GLfloat)camera.window_h, camera.vp_near, 1.5f * camera.vp_far);
			camera.mouse_sens = camera.cam_sensitivity / 3.0f;
		}
		else if (action == GLFW_RELEASE)
		{
			camera.projection = glm::perspective(glm::radians(camera.fov), (GLfloat)camera.window_w / (GLfloat)camera.window_h, camera.vp_near, camera.vp_far);
			camera.mouse_sens = camera.cam_sensitivity;
		}
	}

	// Crouch
	else if (key == GLFW_KEY_C)
	{
		if (action == GLFW_PRESS)
		{
			camera.height = camera.cam_height / 2.0f;
			camera.movement_speed = camera.cam_speed * 0.4f;
		}
		else if (action == GLFW_RELEASE)
		{
			camera.height = camera.cam_height;
			camera.movement_speed = camera.cam_speed;
		}
	}

	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	// Toggle fog
	else if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		static bool draw_fog = false;

		draw_fog = !draw_fog;
		terrain_shader->use();
		terrain_shader->set_bool("drawFog", draw_fog);
		skybox_shader->use();
		skybox_shader->set_bool("drawFog", draw_fog);
		water_shader->use();
		water_shader->set_bool("drawFog", draw_fog);
	}

	// Toggle wave amount
	else if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
	{
		static bool extra_waves = false;

		extra_waves = !extra_waves;
		water_shader->use();
		water_shader->set_bool("extraWaves", extra_waves);
	}

	// Change skybox texture
	else if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
	{
		static unsigned int skybox_index;
		
		skybox_index = ++skybox_index % skybox_paths.size();
		load_cubemap(skybox_paths.at(skybox_index));
	}

	// Toggle flight/walk mode
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
		camera.flying = !camera.flying;
}


// Called on mouse movement
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	static float mouse_last_x = 0.0f;
	static float mouse_last_y = camera.window_h / 2.0f;

	float x_offset = (float)xpos - mouse_last_x;
	float y_offset = mouse_last_y - (float)ypos; // reversed since y-coordinates go from bottom to top

	mouse_last_x = (float)xpos;
	mouse_last_y = (float)ypos;

	camera.process_mouse_movement(x_offset, y_offset);
}


// fb_size_callback is called when the window is resized and updates the projection matrix and viewport size
static void fb_size_callback(GLFWwindow* window, int width, int height)
{
	camera.window_w = width;
	camera.window_h = height;
	camera.projection = glm::perspective(glm::radians(camera.fov), (GLfloat)width / (GLfloat)height, camera.vp_near, camera.vp_far);
	glViewport(0, 0, width, height);
}


// error_callback is called on each glfw error, upon which it displays an error code and description
static void error_callback(int code, const char* description)
{
	std::cerr << "GLFW error " << code << " : " << description << std::endl;
}


/* OpenGL debug callback modified for Odyssey, based on code by Joey de Vries: https://learnopengl.com */
void APIENTRY debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
	// Ignore non-significant error codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API\n"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window system\n"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader compiler\n"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third party\n"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application\n"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other\n"; break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error\n"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated behaviour\n"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined behaviour\n"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability\n"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance\n"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker\n"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push group\n"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop group\n"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other\n"; break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: High\n"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: Medium\n"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: Low\n"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: Notification\n"; break;
	}
	std::cout << std::endl;
}
