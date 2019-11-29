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
	if (!drawFog) {
		// ------------------ Calculate wave effect for normal --------------------
		vec3 normal = vec3(0.0, 1.0, 0.0);

		float normPos = 2.0 * (Position.x + Position.z) / (2048.0 * 8.0 * 2.0); // [0, 2] // TODO: Wrong
		int numWaves = 80;
		float worldPos = mod(normPos * numWaves, 2.0); // [0, 2], repeating numWaves times
		if (worldPos > 1.0)
			worldPos = 2.0 - worldPos; // [0, 1], repeating with discontinuities removed
		normal.x = abs(worldPos * sin(time / 3 + worldPos * 2) / 30); // Position offset used to animate normal
		//normal.z = -abs(worldPos * cos(time / 8 + worldPos) / 32); // TODO: Varför worldPos * sin/cos?
		normal = normalize(normal);

		// ------------- Calculate reflection (in world coordinates) --------------
		vec3 I = normalize(Position - cameraPos);
		vec3 R = reflect(I, normal);
		// Blend cubemap (skybox) reflection and transparent water depending on reflection angle
		float blend = dot(normal, R);
		outColor = (1 - blend) * texture(skybox, R) + blend * vec4(0.21, 0.25, 0.3, 0.75);
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
