#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


/* Shader utility class modified for Odyssey, based on code by Joey de Vries: https://learnopengl.com/Getting-started/Shaders */
class Shader
{
public:
	unsigned int id;
	Shader(const char* vertex_path, const char* fragment_path)
	{
		// 1. Retrieve the vertex/fragment source code from filePath
		std::string vertex_code;
		std::string fragment_code;
		std::ifstream v_shader_file;
		std::ifstream f_shader_file;
		// ensure ifstream objects can throw exceptions (failbit checks fail for some reason):
		v_shader_file.exceptions(std::ifstream::badbit);//| std::ifstream::failbit);
		f_shader_file.exceptions(std::ifstream::badbit);//| std::ifstream::failbit);
		try
		{
			// Open files
			v_shader_file.open(vertex_path);
			f_shader_file.open(fragment_path);
			std::stringstream v_shader_stream, f_shader_stream;
			// Read file's buffer contents into streams
			v_shader_stream << v_shader_file.rdbuf();
			f_shader_stream << f_shader_file.rdbuf();
			// Close file handlers
			v_shader_file.close();
			f_shader_file.close();
			// Convert stream into string
			vertex_code = v_shader_stream.str();
			fragment_code = f_shader_stream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const char* v_shader_code = vertex_code.c_str();
		const char* f_shader_code = fragment_code.c_str();
		// 2. Compile shaders
		unsigned int vertex, fragment;
		// Vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &v_shader_code, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");
		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &f_shader_code, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");
		// Shader Program
		id = glCreateProgram();
		glAttachShader(id, vertex);
		glAttachShader(id, fragment);
		glLinkProgram(id);
		checkCompileErrors(id, "PROGRAM");
		// Delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	// Activate shader
	void use()
	{
		glUseProgram(id);
	}

	/* --------- Uniform loading functions --------- */
	// Utility uniform functions
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
	}

	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}

	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}

	void setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
	}

	// Upload a glm::mat4 to shader program
	void setMatrix4f(const std::string& name, const glm::mat4 matrix) const
	{
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	/* --------- Texture loading functions --------- */
	// Load a texture, using int reference to texture only
	void loadStbTextureRef(const char* filename, GLuint* texture_ref, bool alpha)
	{
		int width_temp, height_temp;
		glGenTextures(1, texture_ref);
		glBindTexture(GL_TEXTURE_2D, *texture_ref);
		// GL_LINEAR_MIPMAP_LINEAR usually seems to be recommended but is slightly more blurry?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
		// Load image, create texture and generate mipmaps
		int nr_channels;
		//stbi_set_flip_vertically_on_load(true); // Tell stb_image.h to flip loaded texture y-axis
		unsigned char* data = stbi_load(filename, &width_temp, &height_temp, &nr_channels, 0);
		if (data)
		{
			if (alpha) // Does the texture have an alpha channel?
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_temp, height_temp, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // Unsure if GL_RGB or GL_RGBA in third argument
			else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_temp, height_temp, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cerr << "Failed to load texture " << filename << std::endl;
		}
		stbi_image_free(data);
	}

private:
	// Utility function for checking shader compilation/linking errors.
	void checkCompileErrors(unsigned int shader, std::string type)
	{
		int success;
		char info_log[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, info_log);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << info_log << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, info_log);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << info_log << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};
