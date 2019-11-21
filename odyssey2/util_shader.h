#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Load_TGA_data.h" // TextureData
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


/* Shader utility class modified for Odyssey, based on code by Joey de Vries: https://learnopengl.com/Getting-started/Shaders */
class Shader
{
public:
	unsigned int ID;
	Shader(const char* vertexPath, const char* fragmentPath)
	{
		// 1. Retrieve the vertex/fragment source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		// ensure ifstream objects can throw exceptions (failbit checks fail for some reason):
		vShaderFile.exceptions(std::ifstream::badbit);//| std::ifstream::failbit);
		fShaderFile.exceptions(std::ifstream::badbit);//| std::ifstream::failbit);
		try
		{
			// Open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// Read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// Close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// Convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();
		// 2. Compile shaders
		unsigned int vertex, fragment;
		// Vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");
		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");
		// Shader Program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");
		// Delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	// Activate shader
	void use()
	{
		glUseProgram(ID);
	}

	/* --------- Uniform loading functions --------- */
	// Utility uniform functions
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}

	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}

	// Upload a glm::mat4 to shader program
	void setMatrix4f(const std::string& name, const glm::mat4 matrix) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	/* --------- Texture loading functions --------- */
	// Load a texture, using int reference to texture only
	void loadStbTextureRef(const char* filename, GLuint* textureRef, bool alpha)
	{
		int width_temp, height_temp;
		loadStbTexture(filename, textureRef, &width_temp, &height_temp, alpha);
	}

	// Load a texture, using texture struct
	void loadStbTextureStruct(const char* filename, TextureData* texture, bool alpha)
	{
		// TODO: Implement this function, properly set TextureData members
		texture->bpp = 8;
		texture->imageData = nullptr;
		loadStbTexture(filename, &texture->texID, (int*)&texture->width, (int *)&texture->height, alpha);
	}

	// Used by loadStbTextureRef and loadStbTextureStruct to load texture.
	void loadStbTexture(const char* filename, GLuint* textureRef, int* width, int* height, bool alpha)
	{
		glGenTextures(1, textureRef);
		glBindTexture(GL_TEXTURE_2D, *textureRef);
		// GL_LINEAR_MIPMAP_LINEAR usually seems to be recommended but is slightly more blurry?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
		// Load image, create texture and generate mipmaps
		int nrChannels;
		//stbi_set_flip_vertically_on_load(true); // Tell stb_image.h to flip loaded texture y-axis
		unsigned char* data = stbi_load(filename, (int *)width, (int *)height, &nrChannels, 0);
		if (data)
		{
			/*  GL_RGB, GL_BGR, GL_RGBA, GL_BGRA
				GL_RGB_INTEGER, GL_BGR_INTEGER, GL_RGBA_INTEGER, GL_BGRA_INTEGER */
			if (alpha) // Does the texture have an alpha channel?
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // Unsure if GL_RGB or GL_RGBA in third argument
			else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, *width, *height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			
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
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};
