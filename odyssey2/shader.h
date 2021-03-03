#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

/* Shader utility class modified for Odyssey, based on code by Joey de Vries: https://learnopengl.com */
class Shader
{
public:
	unsigned int id;
	unsigned int vao; // Vertex array object ID TODO: Move this (and vbo) into models for water and skybox
	Shader(const char* vertex_path, const char* fragment_path);

	// Activate shader
	void use();

	// Utility uniform functions
	void set_bool(const std::string& name, bool value) const;

	void set_int(const std::string& name, int value) const;

	void set_float(const std::string& name, float value) const;

	void set_vec3(const std::string& name, const glm::vec3& value) const;

	void set_vec3(const std::string& name, float x, float y, float z) const;

	void set_mat4_f(const std::string& name, const glm::mat4 matrix) const;

	// Load a texture, using int reference to texture only
	void load_stb_texture_ref(const char* filename, GLuint* texture_ref, bool alpha);

private:
	// Utility function for checking shader compilation/linking errors
	void check_compile_errors(unsigned int shader, std::string type);
};
