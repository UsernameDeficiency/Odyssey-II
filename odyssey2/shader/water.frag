#version 400 core
in vec3 Position;

out vec4 outColor;

uniform samplerCube skybox;
uniform vec3 cameraPos;

void main()
{
	// Calculate reflection (in world coordinates)
	vec3 normal = vec3(0.0, 1.0, 0.0);
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normal);
	/* Blend cubemap (skybox) reflection and transparent water depending on reflection angle
		(The transparent part should be refracted but I'm not sure how to do that) */
	float blend = dot(normal, R);
    outColor = (1 - blend) * texture(skybox, R) + blend * vec4(0.1, 0.2, 0.3, 0.65);
}
