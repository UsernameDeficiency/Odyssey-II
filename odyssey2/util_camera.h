#pragma once
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <unordered_map>
#include "main.h"


/* Camera utility class modified for Odyssey, based on code by Joey de Vries: https://learnopengl.com/Getting-started/Camera
	An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL */
class Camera
{
public:
    // Camera Attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;
	glm::mat4 projection; // Projection matrix
    // Euler Angles
    float yaw;
    float pitch;
    // Default camera values
    int window_w = 1920;
    int window_h = 1080;
    // Camera options
    // Initial camera settings
    const float cam_speed = 120.0f;
    const float cam_sensitivity = 0.2f;
    const float cam_fov = 68.0f; // Vertical field of view (y) in degrees (68 deg vertical = 100 deg horizontal fov)
    const float cam_height = 64.0f; // Camera height above ground
    const float vp_near = 3.0f; // Near distance for frustum
    const float vp_far = 23000.0f; // Far distance for frustum
    // Variable camera settings set by actions like running, flying, zooming 
    float movement_speed;
    float mouse_sens;
	float height;
	bool flying;

    // Keyboard state for controls
    std::unordered_map<int, int> key_state =
    {
        {GLFW_KEY_W, GLFW_RELEASE},
        {GLFW_KEY_A, GLFW_RELEASE},
        {GLFW_KEY_S, GLFW_RELEASE},
        {GLFW_KEY_D, GLFW_RELEASE},
        {GLFW_KEY_Q, GLFW_RELEASE},
        {GLFW_KEY_E, GLFW_RELEASE},
        {GLFW_KEY_C, GLFW_RELEASE},
        {GLFW_KEY_F, GLFW_RELEASE},
        {GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE},
        {GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE},
    };

	Camera();

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 get_view_matrix();

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void process_keyboard(const GLfloat* terrain, const float world_xz_scale, const double delta_time);

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void process_mouse_movement(float x_offset, float y_offset);

private:
	// Interpolate y values over the vertex at position (x, z). No bounds checking for x and z.
    float get_pos_y(float x, float z, const GLfloat* vertex_array, const float world_xz_scale);

    // Calculates the front vector from the Camera's (updated) Euler Angles
    void update_camera_vectors();
};
