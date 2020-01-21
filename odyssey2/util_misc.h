/* Miscellaneous utility functions for the main program */
#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "glm/vec3.hpp"
#include "loadobj.h" // Model*
#include "main.h"

/* Print welcome message */
void greet()
{
	std::cout << "------------------------------------\n"
		"       Welcome to Odyssey II!\n"
		"------------------------------------\n"
		"Move: W/A/S/D/Q/E\n"
		"Run: Shift\n"
		"Zoom: Ctrl\n"
		"Crouch: C\n"
		"Toggle flying/walking: F\n"
		"Toggle fog: F1\n"
		"Toggle water wave effect: F2\n"
		"Toggle skybox: F3\n"
		"------------------------------------\n";
}


/* Print average FPS once every second */
void printFPS()
{
	static unsigned int accFrames;
	static float accTime;

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
float getPosy(float x, float z, GLfloat* vertexArray)
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
	y1 = vertexArray[(xtile + ztile * world_size) * 3 + 1];
	y2 = vertexArray[((xtile + 1) + ztile * world_size) * 3 + 1];
	y3 = vertexArray[(xtile + (ztile + 1) * world_size) * 3 + 1];

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
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API\n"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window system\n"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader compiler\n"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third party\n"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application\n"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other\n"; break;
	}

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error\n"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated behaviour\n"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined behaviour\n"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability\n"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance\n"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker\n"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push group\n"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop group\n"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other\n"; break;
	}

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: High\n"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: Medium\n"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: Low\n"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: Notification\n"; break;
	}
	std::cout << std::endl;
}


/* Exits the program on unrecoverable error, printing an error string to stderr */
void exit_on_error(const char* error)
{
	std::cerr << "Unrecoverable error: " << error << std::endl;
	exit(EXIT_FAILURE);
}


/* Load pre-generated heightmap from texture tex and add height values from procTerrain. */
Model* generateTerrain(std::vector<float> procTerrain, float world_xz_scale, float world_y_scale, float tex_scale)
{
	const int vertexCount = world_size * world_size;
	const int triangleCount = (world_size - 1) * (world_size - 1) * 2;
	unsigned int x, z;
	GLfloat texScaleX = world_size / tex_scale;
	GLfloat texScaleY = world_size / tex_scale;
	GLfloat* vertexArray = new GLfloat[sizeof(GLfloat) * 3 * vertexCount]; // TODO: Array sizes too large here?
	GLfloat* normalArray = new GLfloat[sizeof(GLfloat) * 3 * vertexCount];
	GLfloat* texCoordArray = new GLfloat[sizeof(GLfloat) * 2 * vertexCount];
	GLuint* indexArray = new GLuint[sizeof(GLuint) * triangleCount * 3];

	/* Fill vertex, texture coordinate and index array. */
	for (x = 0; x < world_size; x++) {
		for (z = 0; z < world_size; z++) {
			int index = x + z * world_size;

			// Add pregenerated and diamond square heights
			float y = procTerrain[index] * world_y_scale;
			if (y < minHeight)
				minHeight = y;
			if (y > maxHeight)
				maxHeight = y;

			vertexArray[index * 3 + 0] = x * world_xz_scale;
			vertexArray[index * 3 + 1] = y;
			vertexArray[index * 3 + 2] = z * world_xz_scale;

			/* Scaled texture coordinates. */
			texCoordArray[index * 2 + 0] = (float)x / texScaleX;
			texCoordArray[index * 2 + 1] = (float)z / texScaleY;

			if ((x != world_size - 1) && (z != world_size - 1)) {
				index = (x + z * (world_size - 1)) * 6;
				// Triangle 1
				indexArray[index] = x + z * world_size;
				indexArray[index + 1] = x + (z + 1) * world_size;
				indexArray[index + 2] = x + 1 + z * world_size;
				// Triangle 2
				indexArray[index + 3] = x + 1 + z * world_size;
				indexArray[index + 4] = x + (z + 1) * world_size;
				indexArray[index + 5] = x + 1 + (z + 1) * world_size;
			}
		}
	}

	/* Calculate normals (cross product of two vectors along current triangle) */
	for (x = 0; x < world_size; x++) {
		for (z = 0; z < world_size; z++) {
			int index = (x + z * world_size) * 3;
			// Initialize normals along edges to pointing straight up
			if (x == 0 || (x == world_size - 1) || z == 0 || (z == world_size - 1)) {
				normalArray[index] = 0.0;
				normalArray[index + 1] = 1.0;
				normalArray[index + 2] = 0.0;
			}
			// Inside edges, here the required indices are in bounds
			else {
				glm::vec3 p0(vertexArray[index + world_size * 3], vertexArray[index + 1 + world_size * 3], vertexArray[index + 2 + world_size * 3]);
				glm::vec3 p1(vertexArray[index - world_size * 3], vertexArray[index - world_size * 3 + 1], vertexArray[index - world_size * 3 + 2]);
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
	// Create Model and upload to GPU
	Model* model = LoadDataToModel(
		vertexArray,
		normalArray,
		texCoordArray,
		indexArray,
		vertexCount,
		triangleCount * 3);

	return model;
}


/* Load a cubemap texture. Based on code by Joey de Vries: https://learnopengl.com/Advanced-OpenGL/Cubemaps */
unsigned int loadCubemap(std::vector<std::string> faces)
{
	// Make sure the wanted texture has not already been loaded. This method makes sure new memory is not allocated
	// every time the skybox is changed but still loads the texture from disk every time.
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

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

	return textureID;
}


/* Load skybox texture from the given folder. */
void loadSkyboxTex(std::string skyboxPath)
{
	skyboxShader->use();
	skyboxPath = "tex/skybox/" + skyboxPath;
	std::vector<std::string> faces
	{
		skyboxPath + "/front.tga",
		skyboxPath + "/back.tga",
		skyboxPath + "/top.tga",
		skyboxPath + "/bottom.tga",
		skyboxPath + "/right.tga",
		skyboxPath + "/left.tga"
	};

	skyboxTex = loadCubemap(faces);
	skyboxShader->setInt("skyboxTex", 0);
}
