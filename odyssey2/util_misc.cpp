/* Miscellaneous utility functions for the main program */
#include "util_misc.h"
#include <iostream>
#include <string>
#include <vector>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


/* Load chosen cubemap textures. Based on code by Joey de Vries: https://learnopengl.com/Advanced-OpenGL/Cubemaps */
void load_cubemap(std::vector<GLuint> &skybox_tex)
{
	const std::vector<std::string> skybox_paths = {
		"stormydays", "hw_morning", "sb_frozen", "ame_starfield" };
	skybox_tex.resize(skybox_paths.size());

	for (unsigned int skybox_index{}; skybox_index < skybox_paths.size(); skybox_index++)
	{
		const std::string skybox_path = "tex/skybox/" + skybox_paths.at(skybox_index % skybox_paths.size());
		const std::vector<std::string> faces
		{
			skybox_path + "/front.tga",
			skybox_path + "/back.tga",
			skybox_path + "/top.tga",
			skybox_path + "/bottom.tga",
			skybox_path + "/right.tga",
			skybox_path + "/left.tga"
		};

		glGenTextures(1, &skybox_tex[skybox_index]);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex[skybox_index]);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		int width, height, nr_channels;
		for (unsigned int i = 0; i < faces.size(); i++)
		{
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nr_channels, 0);
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
	}
}


/* Exits the program on unrecoverable error, printing an error string to stderr */
void exit_on_error(const char* error)
{
	std::cerr << "Unrecoverable error: " << error << std::endl;
	exit(EXIT_FAILURE);
}
