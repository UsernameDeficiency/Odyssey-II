#pragma once
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "main.h"

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum class Cam_Movement { CAM_FORWARD, CAM_BACKWARD, CAM_LEFT, CAM_RIGHT, CAM_UP, CAM_DOWN };

// Default camera values
static const float CAM_SPEED = 120.0f;
static const float CAM_SENSITIVITY = 0.2f;
static const float CAM_FOV = 68.0f; // Vertical field of view (y) in degrees (68 deg vertical = 100 deg horizontal fov)
static const float CAM_HEIGHT = 60.0f; // Camera height above ground
static const float VP_NEAR = 3.0f; // Near distance for frustum
static float VP_FAR = 23000.0f; // was world_size * world_xz_scale * 1.4f, probably tweak this later
int window_w = 1920;
int window_h = 1080;

/* Camera utility class modified for Odyssey, based on code by Joey de Vries: https://learnopengl.com/Getting-started/Camera
	An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL */
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
	glm::mat4 projection; // Projection matrix
    // Euler Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float fov;
	float height;
	bool flying;

    // Initialize with glm::vec3 position
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f))
		: Front(glm::vec3(0.0f, 0.0f, -1.0f)), WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
		Yaw(0.0f), Pitch(0.0f), MovementSpeed(CAM_SPEED), MouseSensitivity(CAM_SENSITIVITY), fov(CAM_FOV), height(CAM_HEIGHT), flying(false)
    {
        Position = position;
		projection = glm::perspective(glm::radians(fov), (GLfloat)window_w / (GLfloat)window_h, VP_NEAR, VP_FAR);
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Cam_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
		if (flying)
			velocity *= 4;

		// Update position while making sure that the camera moves in the correct plane
        if (direction == Cam_Movement::CAM_FORWARD)
			Position += glm::normalize(glm::vec3(Front.x, 0.0f, Front.z)) * velocity;
        if (direction == Cam_Movement::CAM_BACKWARD)
            Position -= glm::normalize(glm::vec3(Front.x, 0.0f, Front.z)) * velocity;
        if (direction == Cam_Movement::CAM_LEFT)
            Position -= Right * velocity;
        if (direction == Cam_Movement::CAM_RIGHT)
            Position += Right * velocity;
		if (direction == Cam_Movement::CAM_UP)
			Position += glm::normalize(glm::vec3(0.0f, Up.y, 0.0f)) * velocity;
		if (direction == Cam_Movement::CAM_DOWN)
			Position -= glm::normalize(glm::vec3(0.0f, Up.y, 0.0f)) * velocity;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        // Update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        if (fov >= 1.0f && fov <= 45.0f)
            fov -= yoffset;
        if (fov <= 1.0f)
            fov = 1.0f;
        if (fov >= 45.0f)
            fov = 45.0f;
    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
