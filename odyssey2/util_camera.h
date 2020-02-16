#pragma once
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "main.h"


// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum class cam_movement { CAM_FORWARD, CAM_BACKWARD, CAM_LEFT, CAM_RIGHT, CAM_UP, CAM_DOWN };

// Default camera values
static const float cam_speed = 120.0f;
static const float cam_sensitivity = 0.2f;
static const float cam_fov = 68.0f; // Vertical field of view (y) in degrees (68 deg vertical = 100 deg horizontal fov)
static const float cam_height = 60.0f; // Camera height above ground
static const float vp_near = 3.0f; // Near distance for frustum
static float vp_far = 23000.0f; // was world_size * world_xz_scale * 1.4f, probably tweak this later
int window_w = 1920;
int window_h = 1080;

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
    // Camera options
    float movement_speed;
    float mouse_sens;
    float fov;
	float height;
	bool flying;

    // Initialize with glm::vec3 position
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f))
		: front(glm::vec3(0.0f, 0.0f, -1.0f)), world_up(glm::vec3(0.0f, 1.0f, 0.0f)),
		yaw(0.0f), pitch(0.0f), movement_speed(cam_speed), mouse_sens(cam_sensitivity), fov(cam_fov), height(cam_height), flying(false)
    {
        position = position;
		projection = glm::perspective(glm::radians(fov), (GLfloat)window_w / (GLfloat)window_h, vp_near, vp_far);
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(position, position + front, up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(cam_movement direction, float delta_time)
    {
        float velocity = movement_speed * delta_time;
		if (flying)
			velocity *= 4;

		// Update position while making sure that the camera moves in the correct plane
        if (direction == cam_movement::CAM_FORWARD)
			position += glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * velocity;
        if (direction == cam_movement::CAM_BACKWARD)
            position -= glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * velocity;
        if (direction == cam_movement::CAM_LEFT)
            position -= right * velocity;
        if (direction == cam_movement::CAM_RIGHT)
            position += right * velocity;
		if (direction == cam_movement::CAM_UP)
			position += glm::normalize(glm::vec3(0.0f, up.y, 0.0f)) * velocity;
		if (direction == cam_movement::CAM_DOWN)
			position -= glm::normalize(glm::vec3(0.0f, up.y, 0.0f)) * velocity;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float x_offset, float y_offset)
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
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float y_offset)
    {
        if (fov >= 1.0f && fov <= 45.0f)
            fov -= y_offset;
        if (fov <= 1.0f)
            fov = 1.0f;
        if (fov >= 45.0f)
            fov = 45.0f;
    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
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
};
