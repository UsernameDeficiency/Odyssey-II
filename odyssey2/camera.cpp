#include <glad/glad.h>
#include "camera.h"
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "util_misc.h" // Terrain_heights

extern struct Terrain_heights terrain_struct; // Used by generate_terrain to set heights for water and snow

/* Camera utility class modified for Odyssey, based on code by Joey de Vries: https://learnopengl.com/Getting-started/Camera
	An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL */
Camera::Camera()
{
	projection = glm::perspective(glm::radians(cam_fov), static_cast<float>(window_w) / window_h, vp_near, vp_far);
	update_camera_vectors(); // set front, right, up
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::get_view_matrix()
{
	return glm::lookAt(position, position + front, up);
}

// Processes input received from any keyboard-like input system.
void Camera::process_keyboard(const GLfloat* terrain, const float world_xz_scale, const double delta_time)
{
	float velocity = movement_speed * static_cast<float>(delta_time);
	if (flying)
		velocity *= 4;
	if (key_state[GLFW_KEY_LEFT_SHIFT])
		velocity *= 8;
	if (key_state[GLFW_KEY_C] == GLFW_PRESS || key_state[GLFW_KEY_C] == GLFW_REPEAT)
		velocity /= 2;

	// Crouch
	if (key_state[GLFW_KEY_C] == GLFW_PRESS)
	{
		height = cam_height / 2.0f;
		key_state[GLFW_KEY_C] = GLFW_REPEAT;
	}
	else if (key_state[GLFW_KEY_C] == GLFW_RELEASE)
		height = cam_height;
	
	// Zoom by lowering fov
	if (key_state[GLFW_KEY_LEFT_CONTROL] == GLFW_PRESS)
	{
		projection = glm::perspective(glm::radians(cam_fov / 4), static_cast<float>(window_w) / window_h, vp_near, 1.5f * vp_far);
		mouse_sens = cam_sensitivity / 3.0f;
		key_state[GLFW_KEY_C] = GLFW_REPEAT;
	}
	else if (key_state[GLFW_KEY_LEFT_CONTROL] == GLFW_RELEASE)
	{
		projection = glm::perspective(glm::radians(cam_fov), static_cast<float>(window_w) / window_h, vp_near, vp_far);
		mouse_sens = cam_sensitivity;
	}

	// Toggle flight/walk mode
	if (key_state[GLFW_KEY_F] == GLFW_PRESS)
		flying = !flying;
	key_state[GLFW_KEY_F] = GLFW_REPEAT;

	// Update position while making sure that the camera moves in the correct plane
	if (key_state[GLFW_KEY_W])
		position += glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * velocity;
	if (key_state[GLFW_KEY_S])
		position -= glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * velocity;
	if (key_state[GLFW_KEY_A])
		position -= right * velocity;
	if (key_state[GLFW_KEY_D])
		position += right * velocity;
	if (flying)
	{
		if (key_state[GLFW_KEY_E] && flying)
			position += glm::normalize(glm::vec3(0.0f, up.y, 0.0f)) * velocity;
		if (key_state[GLFW_KEY_Q] && flying)
			position -= glm::normalize(glm::vec3(0.0f, up.y, 0.0f)) * velocity;
	}
	else
	{
		// Move player to terrain height, world_xz_scale padding ensures arrays stay in bound during interpolation
		const float world_limit = world_size * world_xz_scale - world_xz_scale;
		if (position.x < 0.0f)
			position.x = 0.0f;
		else if (position.x > world_limit)
			position.x = world_limit;
		if (position.z < 0.0f)
			position.z = 0.0f;
		else if (position.z > world_limit)
			position.z = world_limit;

		const float new_y_pos = get_pos_y(position.x, position.z, terrain, world_xz_scale) + height;
		const float swim_height = terrain_struct.sea_y_pos + height / 3;

		// Make sure player does not drown.
		if (new_y_pos > swim_height)
			position.y = new_y_pos;
		else
			position.y = swim_height;
	}
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::process_mouse_movement(float x_offset, float y_offset)
{
	x_offset *= mouse_sens;
	y_offset *= mouse_sens;

	yaw += x_offset;
	pitch += y_offset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	// Update front, right and up Vectors using the updated Euler angles
	update_camera_vectors();
}

// Interpolate y values over the vertex at position (x, z). No bounds checking for x and z.
float Camera::get_pos_y(float x, float z, const GLfloat* vertex_array, const float world_xz_scale)
{
	x /= world_xz_scale;
	z /= world_xz_scale;
	int x_tile = static_cast<int>(x);
	int z_tile = static_cast<int>(z);
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

// Calculates the front vector from the Camera's (updated) Euler Angles
void Camera::update_camera_vectors()
{
	// Calculate the new front vector
	glm::vec3 front_tmp{
		cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
		sin(glm::radians(pitch)),
		sin(glm::radians(yaw)) * cos(glm::radians(pitch)) };
	front = glm::normalize(front_tmp);
	// Also re-calculate the right and up vector
	right = glm::normalize(glm::cross(front, world_up)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, front));
}
