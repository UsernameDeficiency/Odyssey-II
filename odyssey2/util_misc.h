/* Miscellaneous utility functions for the main program */
#pragma once
#include <cfloat>
#include <glad/glad.h>
#include <vector>

// Load chosen cubemap textures
// TODO: Move load_cubemap to main.cpp (or to Shader?)
void load_cubemap(std::vector<GLuint>& skybox_tex);

// Exit the program on unrecoverable error, printing an error string to stderr
// TODO: Move exit_on_error to main.cpp
void exit_on_error(const char* error);
