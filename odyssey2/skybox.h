#pragma once
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "shader.h"
#include <glad/glad.h>
#include <vector>

class Skybox
{
public:
	explicit Skybox(const glm::vec3& fog_color);
	~Skybox();

	void bind() const;
	void draw(const glm::mat4& view, const glm::mat4& projection) const;

	// Change to next available texture set, looping back to first set if at end
	void change_active_texture_set();

	// Set if fog effect should be rendered in skybox shader
	void set_fog(bool enabled) const;

private:
	size_t active_texture_index;

	Shader shader;

	std::vector<GLuint> textures;

	GLuint vbo;

	// Load chosen skybox textures
	void load_cubemap();

	void setup_vertices();
};
