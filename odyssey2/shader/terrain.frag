#version 400 core
#define multiTexYLim 0.75
in vec2 passTexCoord;
in vec3 passNormal;
in vec3 phongNormal;
in vec3 pixelPos; // Fragment position in world coordinates

out vec4 outColor;

uniform float seaHeight;
uniform float snowHeight;
uniform sampler2D snowTex;
uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D bottomTex;
uniform mat4 worldToView; // Transform light source to view coords
uniform bool drawFog;
uniform vec3 fogColor;

void main(void)
{
	// ------------------ Lighting --------------------
	// World directional light
	vec3 lightDir = mat3(worldToView) * vec3(1, 0.75, 1); // Direction to light source (sun)

	// Calculate ambient and diffuse light
	float ambient = 0.25;
	float diffuse = max(dot(normalize(lightDir), normalize(phongNormal)), 0.0);
	float shade = ambient + (1 - ambient) * diffuse;
	
	// --------------- Multitexturing -----------------
	// Lake bottom/shoreline
	if (pixelPos.y < seaHeight + 1.0) {
		// Approximate light loss through deep water by gradually darkening fragments
		float depthFac = max((1 + (pixelPos.y - seaHeight) / 256.0f), 0.05f);
		outColor = vec4(shade * depthFac * vec3(texture(bottomTex, passTexCoord)), 1.0);
	}
	else {
		// Rocky slope (underlying ground)
		outColor = vec4(shade * vec3(texture(rockTex, passTexCoord)), 1.0);

		// Blend ground with snow/grass
		if (pixelPos.y < snowHeight && normalize(passNormal).y > multiTexYLim) {
			// Gradually blend between rock/grass depending on angle of the surface and proximity to snow height
			float grassBlend = max(min(8.0f * (normalize(passNormal).y - multiTexYLim) + 1.0f / (pixelPos.y - snowHeight), 1.0f), 0.0f);
			outColor = mix(outColor, vec4(shade * vec3(texture(grassTex, passTexCoord)), 1.0), grassBlend);
		}
		else if (pixelPos.y > snowHeight) {
			// Gradually blend between rock/snow depending on angle of the surface and altitude
			float snowBlend = min((pixelPos.y - snowHeight) * normalize(passNormal).y / 256.0f, 1.0f);
			outColor = mix(outColor, vec4(shade * vec3(texture(snowTex, passTexCoord)), 1.0), snowBlend);
		}
	}
	
	// -------------------- Fog -----------------------
	if (drawFog) {
		float zNear = 3.0f;
		float zFar = 90.0f;
		float z = gl_FragCoord.z * 2.0 - 1.0; // Normalized device coordinates
		float depth = (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
		depth = depth / zFar;
		outColor = vec4(depth * fogColor, depth) + vec4((1 - depth) * vec3(outColor), 1.0);
	}
}
