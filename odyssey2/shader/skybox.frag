#version 400 core
in vec3 TexCoord;

out vec4 outColor;

uniform samplerCube skybox;
uniform int draw_fog;

void main()
{    
    outColor = texture(skybox, TexCoord);

	// Calculate fog
	if (draw_fog == 1)
	{
		float distance = length(vec3(TexCoord.x, 0.5 * TexCoord.y, TexCoord.z));
	    outColor = vec4(vec3(texture(skybox, TexCoord)), distance / 200.0);
	}
}