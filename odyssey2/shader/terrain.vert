#version 400 core
in vec3 inPos;
in vec2 inTexCoord;
in vec3 inNormal;

out vec2 passTexCoord;
out vec3 passNormal;
out vec3 phongNormal;
out vec3 pixelPos; // Fragment position in world coordinates

//uniform mat4 modelToWorld; // Already in world coordinates
uniform mat4 worldToView;
uniform mat4 projection;

void main(void)
{
	mat3 normalMatrix1 = mat3(worldToView);

	// Phong, normal transformation
	phongNormal = inverse(transpose(normalMatrix1)) * inNormal;

	// Direction that the camera is looking
	vec3 player = vec3(normalize(-vec3(worldToView * vec4(inPos, 1.0))));

	passTexCoord = inTexCoord;
	passNormal = inNormal;
	pixelPos = inPos;
	gl_Position = projection * worldToView * vec4(inPos, 1.0);
}
