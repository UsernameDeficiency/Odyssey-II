# Odyssey-II
A significantly improved version of the Odyssey graphics demo

Odyssey II is an OpenGL application that lets the user walk or fly around in a procedurally generated world. The terrain is generated using the diamond-square algorithm and combined with a pre-generated heightmap image as well as a water surface at a fixed height. Other features (will eventually) include procedural multitexturing, cubemap reflections, tesselation, normal mapping and rain simulation.

This project is a continuation of the Odyssey graphics demo. Much of the code has been changed and improved in different ways and the project now uses GLM, GLFW, GLAD and stb_image instead of most of the previous libraries. The repository should contain all files needed for compiling and running an x64 build in Visual Studio 2019. The project has only been tested under Windows so far but should be possible to port to Linux or Mac without too much trouble.

See readme_textures.txt for some notes regarding the bundled textures and skyboxes.
