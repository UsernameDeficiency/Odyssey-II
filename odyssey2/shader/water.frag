#version 400 core
#define M_PI 3.1415926535897932384626433832795
in vec3 Position;

out vec4 outColor;

uniform samplerCube skybox;
uniform vec3 cameraPos;
uniform float time;
uniform bool drawFog;
uniform bool extraWaves;
uniform vec3 fogColor;

void main()
{
	if (!drawFog) {
		// ------------------ Calculate wave effect for normal --------------------
		vec3 normal = vec3(0.0, 1.0, 0.0);
		int numWaves = 96;
		float world_size = 2048.0 * 8.0;

		float normPos = sqrt(Position.x * Position.x + Position.z * Position.z) / (sqrt(2.0) * world_size); // [0, 1]
		normal.x = pow(abs(sin(2 * M_PI * (-time / 10 + normPos * numWaves))), 2) / 64;

		// Add several waves with different velocity and amplitude
		if (extraWaves)
		{
			normPos = Position.z / world_size;
			normal.z += pow(abs(sin(2 * M_PI * (time / 6 + normPos * numWaves * 2 + 0.2))), 2) / 64;

			normPos = sqrt((world_size - Position.x) * (world_size - Position.x) + Position.z * Position.z) / (sqrt(2.0) * world_size);
			normal.z = pow(abs(sin(2 * M_PI * (-time / 7 + normPos * numWaves * 2 + 0.6))), 2) / 96;

			normPos = (Position.x + Position.z) / (2.0 * world_size);
			normal.x += pow(abs(sin(2 * M_PI * (-time / 8 + normPos * numWaves * 2 + 0.8))), 2) / 96;

			//normPos = Position.x / world_size;
			//normal.x += pow(abs(sin(2 * M_PI * (-time / 9 + normPos * numWaves + 0.4))), 2) / 96;
		}

		normal = normalize(normal); // Normal animation finished

		// ------------- Calculate reflection (in world coordinates) --------------
		vec3 I = normalize(Position - cameraPos);
		vec3 R = reflect(I, normal);
		// Blend cubemap (skybox) reflection and transparent water depending on reflection angle
		float blend = pow(dot(normal, R), 2);
		outColor = max((1 - blend), 0.5) * texture(skybox, R);
	}

	else {
		// ---------------------------- Calculate fog -----------------------------
		float zNear = 3.0f;
		float zFar = 90.0f;
		float z = gl_FragCoord.z * 2.0 - 1.0; // Normalized device coordinates
		float depth = (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
		depth = depth / zFar;
		outColor = vec4(depth * fogColor, depth) + (1 - depth) * vec4(0.5, 0.52, 0.56, 0.85);
	}
}
