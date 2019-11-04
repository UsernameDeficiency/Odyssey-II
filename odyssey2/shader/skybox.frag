#version 400 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

// Skybox fragment shader from https://learnopengl.com/Advanced-OpenGL/Cubemaps
void main()
{    
    FragColor = texture(skybox, TexCoords);
}