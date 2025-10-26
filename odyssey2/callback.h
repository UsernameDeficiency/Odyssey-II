/* Callback definitions for GLFW and related functions */
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Handle keyboard actions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Handle mouse movement
void cursor_pos_callback(GLFWwindow* window, double x_pos, double y_pos);

// Handle window resize by updating the projection matrix and viewport size
void fb_size_callback(GLFWwindow* window, int width, int height);

// Display an error code and description on GLFW error
void error_callback(int code, const char* description);

// OpenGL debug callback
void APIENTRY debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param);
