#version 400 core
in vec3 TexCoord;

out vec4 outColor;

uniform samplerCube skybox;

void main()
{    
    outColor = texture(skybox, TexCoord);
}