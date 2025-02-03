/* Callback definitions for GLFW and related functions */
#include "callback.h"
#include "camera.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Handle keyboard actions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	Camera& camera = *static_cast<Camera*>(glfwGetWindowUserPointer(window));
	// Update key state for movement handling if key is bound
	auto search = camera.key_state.find(key);
	if (search != camera.key_state.end())
		camera.key_state[key] = action;
}

// Handle mouse movement
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	Camera& camera = *static_cast<Camera*>(glfwGetWindowUserPointer(window));

	static float mouse_last_x = 0.0f;
	static float mouse_last_y = camera.window_h / 2.0f;

	float x_offset = static_cast<float>(xpos) - mouse_last_x;
	float y_offset = mouse_last_y - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

	mouse_last_x = static_cast<float>(xpos);
	mouse_last_y = static_cast<float>(ypos);

	camera.process_mouse_movement(x_offset, y_offset);
}

// Handle window resize by updating the projection matrix and viewport size
void fb_size_callback(GLFWwindow* window, int width, int height)
{
	Camera& camera = *static_cast<Camera*>(glfwGetWindowUserPointer(window));
	camera.window_w = width;
	camera.window_h = height;
	// Skybox clips if aspect ratio is too wide for chosen vp_near
	camera.projection = glm::perspective(glm::radians(camera.cam_fov), static_cast<float>(width) / height, camera.vp_near, camera.vp_far);
	glViewport(0, 0, width, height);
}

// Display an error code and description on GLFW error
void error_callback(int code, const char* description)
{
	std::cerr << "GLFW error " << code << " : " << description << std::endl;
}

// OpenGL debug callback, based on code by Joey de Vries: https://learnopengl.com
void APIENTRY debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
	// Ignore non-significant error codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API: std::cout << "Source: API\n"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cout << "Source: Window system\n"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader compiler\n"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "Source: Third party\n"; break;
	case GL_DEBUG_SOURCE_APPLICATION: std::cout << "Source: Application\n"; break;
	case GL_DEBUG_SOURCE_OTHER: std::cout << "Source: Other\n"; break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR: std::cout << "Type: Error\n"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated behaviour\n"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "Type: Undefined behaviour\n"; break;
	case GL_DEBUG_TYPE_PORTABILITY: std::cout << "Type: Portability\n"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "Type: Performance\n"; break;
	case GL_DEBUG_TYPE_MARKER: std::cout << "Type: Marker\n"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "Type: Push group\n"; break;
	case GL_DEBUG_TYPE_POP_GROUP: std::cout << "Type: Pop group\n"; break;
	case GL_DEBUG_TYPE_OTHER: std::cout << "Type: Other\n"; break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH: std::cout << "Severity: High\n"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "Severity: Medium\n"; break;
	case GL_DEBUG_SEVERITY_LOW: std::cout << "Severity: Low\n"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: Notification\n"; break;
	}
	std::cout << std::endl;
}
