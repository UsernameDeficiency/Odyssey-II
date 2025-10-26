/* Callback definitions for GLFW and related functions */
#include "callback.h"
#include "camera.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Handle keyboard actions
void key_callback(GLFWwindow* window, const int key, int scancode, const int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	Camera& camera = *static_cast<Camera*>(glfwGetWindowUserPointer(window));
	// Update key state for movement handling if key is bound
	const auto search = camera.key_state.find(key);
	if (search != camera.key_state.end())
		camera.key_state[key] = action;
}

// Handle mouse movement
void cursor_pos_callback(GLFWwindow* window, const double x_pos, const double y_pos)
{
	static double mouse_last_x = x_pos;
	static double mouse_last_y = y_pos;

	const double x_offset = x_pos - mouse_last_x;
	const double y_offset = mouse_last_y - y_pos; // reversed since y-coordinates go from bottom to top

	mouse_last_x = x_pos;
	mouse_last_y = y_pos;

	Camera& camera = *static_cast<Camera*>(glfwGetWindowUserPointer(window));
	camera.process_mouse_movement(static_cast<float>(x_offset), static_cast<float>(y_offset));
}

// Handle window resize by updating the projection matrix and viewport size
void fb_size_callback(GLFWwindow* window, const int width, const int height)
{
	Camera& camera = *static_cast<Camera*>(glfwGetWindowUserPointer(window));
	camera.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	// Skybox clips if aspect ratio is too wide for chosen vp_near
	camera.projection = glm::perspective(glm::radians(camera.cam_fov), camera.aspect_ratio, camera.vp_near, camera.vp_far);
	glViewport(0, 0, width, height);
}

// Display an error code and description on GLFW error
void error_callback(const int code, const char* description)
{
	std::cerr << "GLFW error " << code << " : " << description << '\n';
}

// OpenGL debug callback, based on code by Joey de Vries: https://learnopengl.com
void APIENTRY debug_message(const GLenum source, const GLenum type, const GLuint id, const GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
	// Ignore non-significant error codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	std::cout << "---------------\n";
	std::cout << "Debug message (" << id << "): " << message << '\n';

	std::cout << "Source: ";
	switch (source)
	{
	case GL_DEBUG_SOURCE_API: std::cout << "API\n"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cout << "Window system\n"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Shader compiler\n"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "Third party\n"; break;
	case GL_DEBUG_SOURCE_APPLICATION: std::cout << "Application\n"; break;
	case GL_DEBUG_SOURCE_OTHER: std::cout << "Other\n"; break;
	}

	std::cout << "Type: ";
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR: std::cout << "Error\n"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Deprecated behaviour\n"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "Undefined behaviour\n"; break;
	case GL_DEBUG_TYPE_PORTABILITY: std::cout << "Portability\n"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "Performance\n"; break;
	case GL_DEBUG_TYPE_MARKER: std::cout << "Marker\n"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "Push group\n"; break;
	case GL_DEBUG_TYPE_POP_GROUP: std::cout << "Pop group\n"; break;
	case GL_DEBUG_TYPE_OTHER: std::cout << "Other\n"; break;
	}

	std::cout << "Severity: ";
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH: std::cout << "High\n"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "Medium\n"; break;
	case GL_DEBUG_SEVERITY_LOW: std::cout << "Low\n"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Notification\n"; break;
	}

	std::cout << "---------------\n";
}
