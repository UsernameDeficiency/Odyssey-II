/* Callback definitions for GLFW */
#pragma once
#include "main.h" // Constants and globals
#include "util_misc.h" // get_pos_y


// Keyboard state for key_callback/update_physics
struct
{
	enum { KEY_FORWARD, KEY_BACKWARD, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN };
} key_enum;
bool key_state[key_enum.KEY_DOWN + 1] = { false };


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
		// Move player to terrain height, 2.0f padding ensures arrays stay in bound during interpolation (?)
		if (camera.position.x < 0)
			camera.position.x = 0;
		else if (camera.position.x > world_size* world_xz_scale - 2.0f)
			camera.position.x = world_size * world_xz_scale - 2.0f;
		if (camera.position.z < 0)
			camera.position.z = 0;
		else if (camera.position.z > world_size* world_xz_scale - 2.0f)
			camera.position.z = world_size * world_xz_scale - 2.0f;

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
	{
		if (action == GLFW_PRESS)
			key_state[key_enum.KEY_FORWARD] = true;
		else if (action == GLFW_RELEASE)
			key_state[key_enum.KEY_FORWARD] = false;
	}
	else if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS)
			key_state[key_enum.KEY_BACKWARD] = true;
		else if (action == GLFW_RELEASE)
			key_state[key_enum.KEY_BACKWARD] = false;
	}
	else if (key == GLFW_KEY_A)
	{
		if (action == GLFW_PRESS)
			key_state[key_enum.KEY_LEFT] = true;
		else if (action == GLFW_RELEASE)
			key_state[key_enum.KEY_LEFT] = false;
	}
	else if (key == GLFW_KEY_D)
	{
		if (action == GLFW_PRESS)
			key_state[key_enum.KEY_RIGHT] = true;
		else if (action == GLFW_RELEASE)
			key_state[key_enum.KEY_RIGHT] = false;
	}

	// Move faster is left shift held down
	else if (key == GLFW_KEY_LEFT_SHIFT)
	{
		if (action == GLFW_PRESS)
			camera.movement_speed = camera.cam_speed * 8;
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
			camera.movement_speed = camera.cam_speed * 0.4;
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
		load_skybox_tex(skybox_paths.at(skybox_index));
	}

	// Toggle flight/walk mode
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
		camera.flying = !camera.flying;

	if (camera.flying) // Player flying
	{
		if (key == GLFW_KEY_E)
		{
			if (action == GLFW_PRESS)
				key_state[key_enum.KEY_UP] = true;
			else if (action == GLFW_RELEASE)
				key_state[key_enum.KEY_UP] = false;
		}
		else if (key == GLFW_KEY_Q)
		{
			if (action == GLFW_PRESS)
				key_state[key_enum.KEY_DOWN] = true;
			else if (action == GLFW_RELEASE)
				key_state[key_enum.KEY_DOWN] = false;
		}
	}
	else // Player walking
		key_state[key_enum.KEY_DOWN] = key_state[key_enum.KEY_UP] = false;
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
