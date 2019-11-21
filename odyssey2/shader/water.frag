#version 400 core
in vec3 Position;

out vec4 outColor;

uniform samplerCube skybox;
uniform vec3 cameraPos;
uniform float time;
uniform bool drawFog;
uniform vec3 fogColor;

void main()
{
	// ------------------ Calculate wave effect for normal --------------------
	float normZ = 2.0 * Position.z / (2048.0 * 8.0); // [0, maxAmp]
	int numWaves = 32;
	float worldZ = mod(normZ * numWaves, 2.0); // [0, maxAmp], repeating numWaves times
	if (worldZ > 1.0)
		worldZ = 2.0 - worldZ; // [0, 1], discontinuities removed
	//worldZ -= 0.5; // [-0.5, 0.5] (although this means the "back side" of the waves are visible?)
	float normalPos = worldZ * abs(0.25 * sin(time / 10)); // Position offset used to animate normal
	vec3 normal = normalize(vec3(0.0, 1.0, normalPos));

	// ------------- Calculate reflection (in world coordinates) --------------
	vec3 I = normalize(Position - cameraPos);
	vec3 R = reflect(I, normal);
	// Blend cubemap (skybox) reflection and transparent water depending on reflection angle
	float blend = dot(normal, R);
	outColor = (1 - blend) * texture(skybox, R) + blend * vec4(0.21, 0.25, 0.3, 0.85);
	//outColor = vec4(worldZ * vec3(1.0, 1.0, 1.0), 1.0); // Visualize normal animation

	// ---------------------------- Calculate fog -----------------------------
	if (drawFog)
	{
		float zNear = 3.0f;
		float zFar = 18000.0f / 200.0f;
		float z = gl_FragCoord.z * 2.0 - 1.0; // Normalized device coordinates
		float depth = (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
		depth = depth / zFar;
		outColor = vec4(depth * fogColor, depth) + (1 - depth) * vec4(0.5, 0.52, 0.56, 0.85);
	}
}
