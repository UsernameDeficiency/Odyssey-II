#version 400 core
in vec3 Position;

out vec4 outColor;

uniform samplerCube skybox;
uniform vec3 cameraPos;

void main()
{
	// Calculate fog
//	float zNear = 3.0f;
//	float zFar = 18000.0f/3.0f;
//	float z = gl_FragCoord.z * 2.0 - 1.0; // back to NDC
//	float depth = (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
//	vec4 fog = vec4(0.5 + 0.5*vec3(1 - depth/zFar), 1.0);

	// Calculate reflection (in world coordinates)
	vec3 normal = vec3(0.0, 1.0, 0.0);
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normal);
	/* Blend cubemap (skybox) reflection and transparent water depending on reflection angle
		(The transparent part should be refracted but I'm not sure how to do that) */
	float blend = sqrt(dot(normal, R));
    outColor = (1 - blend) * vec4(vec3(texture(skybox, R)), 1.0) + blend * vec4(0.1, 0.2, 0.3, 0.65);
}

