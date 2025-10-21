#include "camera.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

/* Abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL.
	Based on code by Joey de Vries: https://learnopengl.com/Getting-started/Camera */
Camera::Camera(const float sea_height) : swim_height(sea_height + cam_height / 3)
{
	projection = glm::perspective(glm::radians(cam_fov), aspect_ratio, vp_near, vp_far);
	update_camera_vectors(); // set front, right, up
}

// Return view matrix calculated using Euler Angles and LookAt Matrix
glm::mat4 Camera::get_view_matrix() const
{
	return glm::lookAt(position, position + front, up);
}

// Process input received from keyboard-like input system
void Camera::process_keyboard(const std::vector<GLfloat>& terrain, const float world_xz_scale, const double delta_time)
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
		projection = glm::perspective(glm::radians(cam_fov / 4), aspect_ratio, vp_near, 1.5f * vp_far);
		mouse_sens = cam_sensitivity / 3.0f;
		key_state[GLFW_KEY_C] = GLFW_REPEAT;
	}
	else if (key_state[GLFW_KEY_LEFT_CONTROL] == GLFW_RELEASE)
	{
		projection = glm::perspective(glm::radians(cam_fov), aspect_ratio, vp_near, vp_far);
		mouse_sens = cam_sensitivity;
	}

	// Toggle flight/walk mode
	if (key_state[GLFW_KEY_F] == GLFW_PRESS)
	{
		flying = !flying;
		key_state[GLFW_KEY_F] = GLFW_REPEAT;
	}

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
		if (key_state[GLFW_KEY_E])
			position += glm::normalize(glm::vec3(0.0f, up.y, 0.0f)) * velocity;
		if (key_state[GLFW_KEY_Q])
			position -= glm::normalize(glm::vec3(0.0f, up.y, 0.0f)) * velocity;
	}
	else
		set_pos(terrain, world_xz_scale);
}

// Process input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::process_mouse_movement(const float x_offset, const float y_offset)
{
	yaw += x_offset * mouse_sens;
	pitch += y_offset * mouse_sens;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	// Update front, right and up Vectors using the updated Euler angles
	update_camera_vectors();
}

// Interpolate height over the vertex at position (x, z)
// Interpolations is done over a single triangle, but should probably
// be done over several to smooth transitions between triangles
void Camera::set_pos(const std::vector<GLfloat>& vertex_array, const float world_xz_scale)
{
	// #vertices in 1 dimension (world_size)
	unsigned int num_vert_1D = static_cast<unsigned int>(sqrt(vertex_array.size() / 3));
	// Keep player in bounds, padding ensures arrays stay in bound during interpolation
	const float world_limit = world_xz_scale * (num_vert_1D - 1);
	if (position.x < 0.0f)
		position.x = 0.0f;
	else if (position.x > world_limit)
		position.x = world_limit;
	if (position.z < 0.0f)
		position.z = 0.0f;
	else if (position.z > world_limit)
		position.z = world_limit;

	const float x{ position.x / world_xz_scale };
	const float z{ position.z / world_xz_scale };
	const size_t x_tile{ static_cast<size_t>(x) };
	const size_t z_tile{ static_cast<size_t>(z) };
	const float x_pos{ x - x_tile };
	const float z_pos{ z - z_tile };

	// Interpolate over the triangle at the current player position
	// Interpolation is done into higher x and z
	const float y1 = vertex_array[(x_tile + z_tile * num_vert_1D) * 3 + 1];
	const float y2 = vertex_array[(((x_tile + 1) % num_vert_1D) + z_tile * num_vert_1D) * 3 + 1];
	const float y3 = vertex_array[(x_tile + ((z_tile + 1) % num_vert_1D) * num_vert_1D) * 3 + 1];
	const float y_pos = y1 + x_pos * (y2 - y1) + z_pos * (y3 - y1) + height;

	// Make sure player does not drown
	if (y_pos > swim_height)
		position.y = y_pos;
	else
		position.y = swim_height;
}

// Calculate the front vector from Camera's (updated) Euler Angles
void Camera::update_camera_vectors()
{
	// Calculate the new front vector
	glm::vec3 front_tmp{
		cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
		sin(glm::radians(pitch)),
		sin(glm::radians(yaw)) * cos(glm::radians(pitch))
	};
	front = glm::normalize(front_tmp);
	// Also re-calculate the right and up vector
	// Normalize the vectors, because their length gets closer to 0 the more you look up
	// or down which results in slower movement
	right = glm::normalize(glm::cross(front, world_up));
	up = glm::normalize(glm::cross(right, front));
}
