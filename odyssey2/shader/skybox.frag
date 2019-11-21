#version 400 core
in vec3 TexCoord;

out vec4 outColor;

uniform samplerCube skybox;
uniform int drawFog;

void main()
{    
    outColor = texture(skybox, TexCoord);

	// Calculate fog
	if (drawFog == 1)
	{
		float distance = length(vec3(TexCoord.x, 0.5 * TexCoord.y, TexCoord.z));
	    outColor = vec4(vec3(texture(skybox, TexCoord)), distance / 200.0);
	}
}