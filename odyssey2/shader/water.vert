#version 400 core
in vec3 inPos;

out vec3 Position;

//uniform mat4 model; // Already in world coordinates
uniform mat4 worldToView;
uniform mat4 projection;

void main()
{
	Position = inPos;
	gl_Position = projection * worldToView * vec4(inPos, 1.0);
}
