#version 400 core
in vec3 inPos;

out vec3 Position;

//uniform mat4 model; // Already in world coordinates
uniform mat4 worldToView;
uniform mat4 projection;
uniform float time;

void main()
{
	// Animate water vertices in y direction
	vec3 inPosAnim = inPos;
	inPosAnim.y += sin(time + inPos.x) / 4;
	inPosAnim.y += sin(time + inPos.y) / 4;

	Position = inPosAnim;
	gl_Position = projection * worldToView * vec4(inPosAnim, 1.0);
}
