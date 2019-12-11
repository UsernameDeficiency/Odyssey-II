#version 400 core
in vec3 TexCoord;

out vec4 outColor;

uniform samplerCube skyboxTex;
uniform bool drawFog;

void main()
{    
    outColor = texture(skyboxTex, TexCoord);

	// Calculate fog
	if (drawFog)
	{
		float distance = length(vec3(TexCoord.x, 0.5 * TexCoord.y, TexCoord.z));
	    outColor = vec4(vec3(texture(skyboxTex, TexCoord)), distance / 300.0);
	}
}