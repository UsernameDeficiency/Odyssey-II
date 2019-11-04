#version 400 core

in vec3 in_Position;

uniform mat4 worldToView;
uniform mat4 modelToWorld;
uniform mat4 projection;

void main(void)
{
	gl_Position = projection * worldToView * modelToWorld * vec4(in_Position, 1.0);
}
