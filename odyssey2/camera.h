#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <unordered_map>
#include <vector>

/* Abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL.
	Based on code by Joey de Vries: https://learnopengl.com/Getting-started/Camera */
class Camera
{
public:
	// Camera Attributes
	glm::vec3 position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up{ 0.0f, 1.0f, 0.0f };
	glm::mat4 projection; // Projection matrix
	// Euler Angles
	float yaw{ 0.0f };
	float pitch{ 0.0f };
	// Aspect ratio for projection matrix
	// Should be updated using actual window size after window is created or resized
	float aspect_ratio{ 16.0f / 9.0f };
	// Camera options
	// Initial camera settings
	const float cam_fov; // Vertical field of view (y) in degrees (68 deg vertical = 100 deg horizontal fov)
	const float cam_height; // Camera height above ground
	const float cam_sensitivity;
	const float cam_speed;
	const float vp_near; // Near distance for frustum, lower this if skybox is cut off at screen edges
	const float vp_far; // Far distance for frustum
	// Minimum height, keeps camera above water level
	const float swim_height;
	// Variable camera settings set by actions like running, flying, zooming
	float movement_speed{ cam_speed };
	float mouse_sens{ cam_sensitivity };
	float height;
	bool flying{ false };

	// Keyboard state for controls
	std::unordered_map<int, int> key_state = {
		{ GLFW_KEY_W, GLFW_RELEASE },
		{ GLFW_KEY_A, GLFW_RELEASE },
		{ GLFW_KEY_S, GLFW_RELEASE },
		{ GLFW_KEY_D, GLFW_RELEASE },
		{ GLFW_KEY_Q, GLFW_RELEASE },
		{ GLFW_KEY_E, GLFW_RELEASE },
		{ GLFW_KEY_C, GLFW_RELEASE },
		{ GLFW_KEY_F, GLFW_RELEASE },
		{ GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE },
		{ GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE },
		{ GLFW_KEY_F1, GLFW_RELEASE },
		{ GLFW_KEY_F2, GLFW_RELEASE },
		{ GLFW_KEY_F3, GLFW_RELEASE }
	};

	Camera(const float sea_height);

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 get_view_matrix() const;

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void process_keyboard(const std::vector<GLfloat>& terrain, const float world_xz_scale, const double delta_time);

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void process_mouse_movement(const float x_offset, const float y_offset);

private:
	// Interpolate height over the vertex at position (x, z)
	void set_pos(const std::vector<GLfloat>& vertex_array, const float world_xz_scale);

	// Calculates the front vector from the Camera's (updated) Euler Angles
	void update_camera_vectors();
};
