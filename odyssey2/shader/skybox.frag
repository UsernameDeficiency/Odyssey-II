#version 400 core
in vec3 TexCoord;

out vec4 outColor;

uniform samplerCube skyboxTex;
uniform bool drawFog;

void main()
{
	if (!drawFog)
		outColor = texture(skyboxTex, TexCoord);
	// else, show background color (fog)
}