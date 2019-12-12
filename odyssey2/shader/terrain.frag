#version 400 core
#define multiTexYLim 0.8
#define snowHeight -900
#define watersideOffset 0.5
in vec2 passTexCoord;
in vec3 passNormal;
in vec3 phongNormal;
in vec3 pixelPos; // Fragment position in world coordinates

out vec4 outColor;

uniform float seaLevel;
uniform sampler2D snowTex;
uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D bottomTex;
uniform mat4 worldToView; // Transform light source to view coords
uniform bool drawFog;
uniform vec3 fogColor;

void main(void)
{
	// World directional light
	vec3 lightDir = mat3(worldToView) * vec3(1, 0.75, 1); // Direction to light source (sun)

	// Calculate ambient and diffuse light
	float ambient = 0.3;
	float diffuse = max(dot(normalize(lightDir), normalize(phongNormal)), 0.0);
	float shade = ambient + 0.8*diffuse;

	// TODO: Modulate multitexturing limits with gl_FragCoord to make limits less visible
	// Lake bottom
	if (pixelPos.y < seaLevel + watersideOffset) {
		outColor = vec4(shade * vec3(texture(bottomTex, passTexCoord)), 1.0);
	}
	else if (normalize(passNormal).y > multiTexYLim) {
		// Grass
		if (pixelPos.y < snowHeight)
			outColor = vec4(shade * vec3(texture(grassTex, passTexCoord)), 1.0);
		// Snow TODO: use mix() to blend between snow and underlying ground? Maybe use normal value to give gradual change
		else
			outColor = vec4(shade * vec3(texture(snowTex, passTexCoord)), 1.0);
	}
	// Rocky slope
	else {
		outColor = vec4(shade * vec3(texture(rockTex, passTexCoord)), 1.0);
	}

	// Calculate fog
	if (drawFog) {
		float zNear = 3.0f;
		float zFar = 90.0f;
		float z = gl_FragCoord.z * 2.0 - 1.0; // Normalized device coordinates
		float depth = (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
		depth = depth / zFar;
		outColor = vec4(depth * fogColor, depth) + vec4((1 - depth) * vec3(outColor), 1.0);
	}
}
