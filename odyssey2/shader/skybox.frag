#version 400 core
in vec3 TexCoord;

out vec4 outColor;

uniform samplerCube skyboxTex;
uniform bool drawFog;
uniform vec3 fogColor;

void main()
{
	if (!drawFog)
		outColor = texture(skyboxTex, TexCoord);
	else
		outColor = vec4(fogColor, 1.0f);
}