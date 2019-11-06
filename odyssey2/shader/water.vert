#version 400 core

in vec3 inPos;

//uniform mat4 modelToWorld; // Already in world coordinates
uniform mat4 worldToView;
uniform mat4 projection;

void main(void)
{
	gl_Position = projection * worldToView * vec4(inPos, 1.0);
}
