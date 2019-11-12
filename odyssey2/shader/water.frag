#version 400 core
in vec3 Position;

out vec4 outColor;

uniform samplerCube skybox;
uniform vec3 cameraPos;
//uniform float time;
uniform int draw_fog;

void main()
{
	// TODO: Animate normal vector to give appearance of waves
	vec3 normal = vec3(0.0, 1.0, 0.0);

	// Calculate reflection (in world coordinates)
	vec3 I = normalize(Position - cameraPos);
	vec3 R = reflect(I, normal);
	/* Blend cubemap (skybox) reflection and transparent water depending on reflection angle */
	float blend = dot(normal, R);
	outColor = (1 - blend) * texture(skybox, R) + blend * vec4(0.21, 0.25, 0.3, 0.85);

	// Calculate fog
	if (draw_fog == 1)
	{
		float zNear = 3.0f;
		float zFar = 18000.0f / 128.0f;
		float z = gl_FragCoord.z * 2.0 - 1.0; // Normalized device coordinates
		float depth = (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
		depth = depth / zFar;
		depth = sqrt(depth);
		vec3 fog_color = 0.7 * vec3(1.0, 1.0, 1.0);
		outColor = vec4(depth * fog_color, depth) + (1 - depth) * vec4(0.5, 0.52, 0.56, 0.85);
	}
}

