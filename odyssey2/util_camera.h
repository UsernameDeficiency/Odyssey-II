#pragma once
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
    const float cam_speed = 120.0f;
    const float cam_sensitivity = 0.2f;
    const float cam_fov = 68.0f; // Vertical field of view (y) in degrees (68 deg vertical = 100 deg horizontal fov)
    const float cam_height = 60.0f; // Camera height above ground
    const float vp_near = 3.0f; // Near distance for frustum
    const float vp_far = 23000.0f; // Far distance for frustum
    float movement_speed;
    float mouse_sens;
    float fov;
	float height;
	bool flying;

    // Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
    enum class movement { CAM_FORWARD, CAM_BACKWARD, CAM_LEFT, CAM_RIGHT, CAM_UP, CAM_DOWN };

    // Initialize with glm::vec3 position
    Camera()
		: position{ 0.0f, 0.0f, 0.0f }, front{ 0.0f, 0.0f, -1.0f }, world_up{ 0.0f, 1.0f, 0.0f },
		yaw(0.0f), pitch(0.0f), movement_speed(cam_speed), mouse_sens(cam_sensitivity), 
        fov(cam_fov), height(cam_height), flying(false)
    {
		projection = glm::perspective(glm::radians(fov), (GLfloat)window_w / (GLfloat)window_h, vp_near, vp_far);
        update_camera_vectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 get_view_matrix()
    {
        return glm::lookAt(position, position + front, up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void process_keyboard(Camera::movement direction, float delta_time)
    {
        float velocity = movement_speed * delta_time;
        if (flying)
            velocity *= 4;

        // Update position while making sure that the camera moves in the correct plane
        if (direction == Camera::movement::CAM_FORWARD)
            position += glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * velocity;
        if (direction == Camera::movement::CAM_BACKWARD)
            position -= glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * velocity;
        if (direction == Camera::movement::CAM_LEFT)
            position -= right * velocity;
        if (direction == Camera::movement::CAM_RIGHT)
            position += right * velocity;
        if (direction == Camera::movement::CAM_UP)
            position += glm::normalize(glm::vec3(0.0f, up.y, 0.0f)) * velocity;
        if (direction == Camera::movement::CAM_DOWN)
			position -= glm::normalize(glm::vec3(0.0f, up.y, 0.0f)) * velocity;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void process_mouse_movement(float x_offset, float y_offset)
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

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void process_mouse_scroll(float y_offset)
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
    void update_camera_vectors()
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
