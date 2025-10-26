#include "skybox.h"
#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/vector_float3.hpp"
#include "shader.h"
#include <stb_image.h>
#include <stdexcept>
#include <vector>

Skybox::Skybox(const glm::vec3& fog_color)
	: active_texture_index{}, shader(Shader{ "shader/skybox.vert", "shader/skybox.frag" }), vbo{}
{
	shader.use();
	shader.set_bool("drawFog", false);
	shader.set_vec3("fogColor", fog_color);
	load_cubemap();
	shader.set_int("skyboxTex", 0);
	setup_vertices();
}

Skybox::~Skybox()
{
	glDeleteBuffers(1, &vbo);
	for (auto tex : textures)
		glDeleteTextures(1, &tex);
}

// Load all sets of skybox textures
void Skybox::load_cubemap()
{
	// TODO: Move skybox textures into settings file. How to create a vector from the settings file?
	const std::vector<std::string> skybox_paths = {
		"stormydays", "hw_morning", "sb_frozen", "ame_starfield"
	};
	textures.resize(skybox_paths.size());

	for (unsigned int skybox_index{}; skybox_index < skybox_paths.size(); skybox_index++)
	{
		const std::string skybox_path = "tex/skybox/" + skybox_paths.at(skybox_index);
		const std::vector<std::string> faces{
			skybox_path + "/front.tga",
			skybox_path + "/back.tga",
			skybox_path + "/top.tga",
			skybox_path + "/bottom.tga",
			skybox_path + "/right.tga",
			skybox_path + "/left.tga"
		};

		glGenTextures(1, &textures[skybox_index]);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textures[skybox_index]);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		int width, height, nr_channels;
		for (size_t face_index{}; face_index < faces.size(); face_index++)
		{
			unsigned char* data = stbi_load(faces[face_index].c_str(), &width, &height, &nr_channels, 0);
			if (!data)
			{
				// Clean up texture set that failed to load. Might leak previously loaded texture sets.
				glDeleteTextures(1, &textures[skybox_index]);
				throw std::runtime_error("Failed to load skybox texture " + faces[face_index] + ": " + stbi_failure_reason());
			}
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_index,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
	}
}

void Skybox::setup_vertices()
{
	constexpr GLfloat skybox_vertices[]{
		-5.0f, 5.0f, -5.0f, -5.0f, -5.0f, -5.0f, 5.0f, -5.0f, -5.0f,
		5.0f, -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, -5.0f, 5.0f, -5.0f,

		-5.0f, -5.0f, 5.0f, -5.0f, -5.0f, -5.0f, -5.0f, 5.0f, -5.0f,
		-5.0f, 5.0f, -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, -5.0f, 5.0f,

		5.0f, -5.0f, -5.0f, 5.0f, -5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
		5.0f, 5.0f, 5.0f, 5.0f, 5.0f, -5.0f, 5.0f, -5.0f, -5.0f,

		-5.0f, -5.0f, 5.0f, -5.0f, 5.0f, 5.0f, 5.0f, 5.0f, 5.0f,
		5.0f, 5.0f, 5.0f, 5.0f, -5.0f, 5.0f, -5.0f, -5.0f, 5.0f,

		-5.0f, 5.0f, -5.0f, 5.0f, 5.0f, -5.0f, 5.0f, 5.0f, 5.0f,
		5.0f, 5.0f, 5.0f, -5.0f, 5.0f, 5.0f, -5.0f, 5.0f, -5.0f,

		-5.0f, -5.0f, -5.0f, -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, -5.0f,
		5.0f, -5.0f, -5.0f, -5.0f, -5.0f, 5.0f, 5.0f, -5.0f, 5.0f
	};
	// vao is handled in shader
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
}

void Skybox::bind() const
{
	glBindVertexArray(shader.vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textures[active_texture_index]);
}

void Skybox::draw(const glm::mat4& view_matrix, const glm::mat4& projection) const
{
	glDepthMask(GL_FALSE); // Disable depth writes
	shader.use();
	bind();
	const auto world_to_view{ glm::mat4(glm::mat3(view_matrix)) }; // Remove translation from the view matrix
	shader.set_mat4_f("worldToView", world_to_view);
	shader.set_mat4_f("projection", projection);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
}

void Skybox::change_active_texture_set()
{
	active_texture_index = (active_texture_index + 1) % textures.size();
}

void Skybox::set_fog(const bool enabled) const
{
	shader.use();
	shader.set_bool("drawFog", enabled);
}
