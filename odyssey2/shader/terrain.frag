#version 400 core
#define multiTexYLim 0.8
#define multiTexWaterside 0.5

in vec2 passTexCoord;
in vec3 passNormal;
in vec3 phongNormal;
in vec3 pixelPos; // Fragment position in world coordinates

out vec4 outColor;

uniform float seaLevel;
uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D bottomTex;
uniform mat4 worldToView; // Transform light source to view coords

void main(void)
{
	// World directional light
	vec3 lightDir = mat3(worldToView) * vec3(1, 0.75, 1); // Direction to light source (sun)

	// Calculate ambient and diffuse light
	float ambient = 0.2;
	float diffuse = max(dot(normalize(lightDir), normalize(phongNormal)), 0.0);
	float shade = ambient + 0.8*diffuse;

	// Lake bottom
	if (pixelPos.y < seaLevel + multiTexWaterside) {
		outColor = vec4(shade * vec3(texture(bottomTex, passTexCoord)), 1.0);
	}
	// Grass
	else if (normalize(passNormal).y > multiTexYLim) {
		outColor = vec4(shade * vec3(texture(grassTex, passTexCoord)), 1.0);
	}
	// Rocky slope
	else {
		outColor = vec4(shade * vec3(texture(rockTex, passTexCoord)), 1.0);
	}
}
