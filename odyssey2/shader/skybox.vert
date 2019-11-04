#version 400 core
layout (location = 0) in vec3 inPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 worldToView;

// Skybox vertex shader from https://learnopengl.com/Advanced-OpenGL/Cubemaps
void main()
{
    TexCoords = inPos;
    gl_Position = projection * worldToView * vec4(inPos, 1.0);
} 