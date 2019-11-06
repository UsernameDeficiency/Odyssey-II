#version 400 core
in vec3 inPos;

out vec3 TexCoord;

uniform mat4 projection;
uniform mat4 worldToView;

void main()
{
    TexCoord = inPos;
    gl_Position = projection * worldToView * vec4(inPos, 1.0);
} 