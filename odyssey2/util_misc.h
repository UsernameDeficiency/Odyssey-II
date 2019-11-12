/* Miscellaneous utility functions for the main program */
#pragma once
#include "load_TGA_data.h" // TextureData*
#include "util_shader.h"
#include "loadobj.h" // Model*
#include "glm/vec3.hpp"
#include <iostream>
#include <vector>
#include <string>


/* Print welcome message */
void greet()
{
	std::cout << "------------------------------------\n"
		"       Welcome to Odyssey II!\n"
		"------------------------------------\n"
		"The following controls are available\n"
		"Move: W/A/S/D/Q/E\n"
		"Run: Shift\n"
		"Zoom: Ctrl\n"
		"Crouch: C\n"
		"Toggle flying/walking: F\n"
		"Toggle fog: F1\n"
		"------------------------------------\n"
		"Loading";
}


/* Print average FPS once every second */
void printFPS()
{
	accTime += deltaTime;
	accFrames++;

	if (accTime > 1.0f)
	{
		std::cout << "FPS: " << accFrames / accTime << "\n";
		accTime = 0.0f;
		accFrames = 0;
	}
}


/* Interpolate y values over the vertex at position (x, z). No bounds checking for x and z. */
float getPosy(float x, float z, GLfloat* vertexArray, TextureData* tex)
{
	float yPos, y1, y2, y3;
	x /= world_xz_scale;
	z /= world_xz_scale;
	int xtile = (int)x;
	int ztile = (int)z;
	float xPos = (x - xtile);
	float zPos = (z - ztile);

	// Interpolate over the triangle at the current player position.
	// Interpolation is done into higher x and z, therefore x and z must be below upper array bounds
	y1 = vertexArray[(xtile + ztile * tex->width) * 3 + 1];
	y2 = vertexArray[((xtile + 1) + ztile * tex->width) * 3 + 1];
	y3 = vertexArray[(xtile + (ztile + 1) * tex->width) * 3 + 1];

	yPos = y1 + xPos * (y2 - y1) + zPos * (y3 - y1);

	return yPos;
}


/* OpenGL debug callback modified for Odyssey, based on code by Joey de Vries: https://learnopengl.com */
void APIENTRY debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	// Ignore non-significant error codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window system"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: High"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: Medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: Low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: Notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}


/* Exits the program on unrecoverable error, printing an error string to stderr */
void exit_on_error(const char* error)
{
	std::cerr << "Unrecoverable error: " << error << std::endl;
	exit(EXIT_FAILURE);
}


/* Load pre-generated heightmap from texture tex and add height values from procTerrain. */
Model* generateTerrain(TextureData* tex, float* procTerrain, float world_xz_scale, float world_y_scale, float tex_scale)
{
	const int vertexCount = tex->width * tex->height;
	const int triangleCount = (tex->width - 1) * (tex->height - 1) * 2;
	unsigned int x, z;
	GLfloat texScaleX = tex->width / tex_scale;
	GLfloat texScaleY = tex->height / tex_scale;
	GLfloat* vertexArray = new GLfloat[sizeof(GLfloat) * 3 * vertexCount];
	GLfloat* normalArray = new GLfloat[sizeof(GLfloat) * 3 * vertexCount];
	GLfloat* texCoordArray = new GLfloat[sizeof(GLfloat) * 2 * vertexCount];
	GLuint* indexArray = new GLuint[sizeof(GLuint) * triangleCount * 3];
	float meanHeight = 0; // Used to set sea_y_pos

	/* Fill vertex, texture coordinate and index array. */
	for (x = 0; x < tex->width; x++) {
		for (z = 0; z < tex->height; z++) {
			int index = x + z * tex->width;

			// Add pregenerated and diamond square heights
			float y = (tex->imageData[index * (tex->bpp / 8)] + procTerrain[index]) * world_y_scale;
			meanHeight += y;

			vertexArray[index * 3 + 0] = x * world_xz_scale;
			vertexArray[index * 3 + 1] = y;
			vertexArray[index * 3 + 2] = z * world_xz_scale;

			/* Scaled texture coordinates. */
			texCoordArray[index * 2 + 0] = (float)x / texScaleX;
			texCoordArray[index * 2 + 1] = (float)z / texScaleY;

			if ((x != tex->width - 1) && (z != tex->width - 1)) {
				index = (x + z * (tex->width - 1)) * 6;
				// Triangle 1
				indexArray[index] = x + z * tex->width;
				indexArray[index + 1] = x + (z + 1) * tex->width;
				indexArray[index + 2] = x + 1 + z * tex->width;
				// Triangle 2
				indexArray[index + 3] = x + 1 + z * tex->width;
				indexArray[index + 4] = x + (z + 1) * tex->width;
				indexArray[index + 5] = x + 1 + (z + 1) * tex->width;
			}
		}
	}

	/* Calculate normals (cross product of two vectors along current triangle) */
	for (x = 0; x < tex->width; x++) {
		for (z = 0; z < tex->height; z++) {
			int index = (x + z * tex->width) * 3;
			// Initialize normals along edges to pointing straight up
			if (x == 0 || (x == tex->width - 1) || z == 0 || (z == tex->width - 1)) {
				normalArray[index] = 0.0;
				normalArray[index + 1] = 1.0;
				normalArray[index + 2] = 0.0;
			}
			// Inside edges, here the required indices are in bounds
			else {
				glm::vec3 p0(vertexArray[index + tex->width * 3], vertexArray[index + 1 + tex->width * 3], vertexArray[index + 2 + tex->width * 3]);
				glm::vec3 p1(vertexArray[index - tex->width * 3], vertexArray[index - tex->width * 3 + 1], vertexArray[index - tex->width * 3 + 2]);
				glm::vec3 p2(vertexArray[index - 3], vertexArray[index - 2], vertexArray[index - 1]);
				glm::vec3 a(p1 - p0);
				glm::vec3 b(p2 - p0);
				glm::vec3 normal = glm::cross(a, b);

				normalArray[index] = normal.x;
				normalArray[index + 1] = normal.y;
				normalArray[index + 2] = normal.z;
			}
		}
	}
	// Create Model and upload to GPU:
	Model* model = LoadDataToModel(
		vertexArray,
		normalArray,
		texCoordArray,
		NULL,
		indexArray,
		vertexCount,
		triangleCount * 3);

	if (meanHeight > 0)
		sea_y_pos = 0.80f * meanHeight / (tex->width * tex->width);
	else
		sea_y_pos = 1.25f * meanHeight / (tex->width * tex->width);
	return model;
}


/* Load a cubemap texture. Based on code by Joey de Vries: https://learnopengl.com/Advanced-OpenGL/Cubemaps */
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (!data)
		{
			std::cerr << "loadCubemap failed: texture " << faces[i] << " failed to load\n";
			stbi_image_free(data);
			exit(EXIT_FAILURE);
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}