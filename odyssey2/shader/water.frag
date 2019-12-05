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
		int numWaves = 80;
		float world_size = 2048.0 * 8.0;

		float normPos = sqrt(Position.x * Position.x + Position.z * Position.z) / (sqrt(2.0) * world_size); // [0, 1]
		normal.x = abs(sin(2 * M_PI * (-time / 16 + normPos * numWaves)) / 50);

		if (extraWaves)
		{
			normPos = sqrt((world_size - Position.x) * (world_size - Position.x) + Position.z * Position.z) / (sqrt(2.0) * world_size);
			normal.z = (cos(2 * M_PI * (-time / 8 + normPos * numWaves * 4)) / 80);

			normPos = Position.x / world_size;
			normal.x += (sin(2 * M_PI * (-time / 7 + normPos * numWaves * 2)) / 64);

			normPos = Position.z / world_size;
			normal.z += (cos(2 * M_PI * (-time / 8 + normPos * numWaves)) / 64);

			normPos = (Position.x + Position.z) / (2.0 * world_size);
			normal.x += sin(2 * M_PI * (time / 9 + normPos * numWaves * 4)) / 128;
		}

		normal = normalize(normal); // Normal animation finished

		// ------------- Calculate reflection (in world coordinates) --------------
		vec3 I = normalize(Position - cameraPos);
		vec3 R = reflect(I, normal);
		// Blend cubemap (skybox) reflection and transparent water depending on reflection angle
		float blend = dot(normal, R);
		outColor = (1 - blend) * texture(skybox, R) + blend * vec4(0.21, 0.25, 0.3, 0.75);
		//outColor = vec4(20*normal.x, 0.0, 20*normal.z, 1.0); // Normal animation debug
	}

	else {
		// ---------------------------- Calculate fog -----------------------------
		float zNear = 3.0f;
		float zFar = 18000.0f / 200.0f;
		float z = gl_FragCoord.z * 2.0 - 1.0; // Normalized device coordinates
		float depth = (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
		depth = depth / zFar;
		outColor = vec4(depth * fogColor, depth) + (1 - depth) * vec4(0.5, 0.52, 0.56, 0.85);
	}
}
