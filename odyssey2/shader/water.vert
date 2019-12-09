#version 400 core
#define M_PI 3.1415926535897932384626433832795
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
	inPosAnim.y += sin(2 * M_PI * (time / 3)) / 12;
	inPosAnim.y += sin(2 * M_PI * (time / 3 + 1 / 6)) / 12;
	inPosAnim.y += sin(2 * M_PI * (time / 6) + 2 / 6) / 8;
	inPosAnim.y += sin(2 * M_PI * (time / 6 + 3 / 6)) / 8;
	inPosAnim.y += sin(2 * M_PI * (time / 10 + 4 / 6)) / 6;

	Position = inPosAnim;
	gl_Position = projection * worldToView * vec4(inPosAnim, 1.0);
}
