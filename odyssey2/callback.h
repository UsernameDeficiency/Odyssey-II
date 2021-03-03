/* Callback definitions for GLFW and related functions */
#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Handle keyboard actions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Called on mouse movement
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);

// Called when the window is resized and updates the projection matrix and viewport size
void fb_size_callback(GLFWwindow* window, int width, int height);

// Called on each glfw error, upon which it displays an error code and description
void error_callback(int code, const char* description);

// OpenGL debug callback
void APIENTRY debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param);
