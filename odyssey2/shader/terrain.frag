#version 400 core
#define multiTexYLim 0.8

in vec2 passTexCoord;
in vec3 passNormal;
in vec3 phongNormal;
in vec3 light;
in vec3 pixelPos; // Fragment position in world coordinates

out vec4 outColor;

uniform float seaLevel;
uniform sampler2D grassTex;
uniform sampler2D rockTex;
uniform sampler2D bottomTex;

void main(void)
{
	// Calculate ambient and diffuse light
	float ambient = 0.0;
	float diffuse = max(dot(normalize(light), normalize(phongNormal)), 0.0);
	float shade = ambient + 0.7*diffuse;

	// Lake bottom
	if (pixelPos.y < seaLevel + 0.2) {
		outColor = vec4(shade*vec3(texture(bottomTex, passTexCoord)), 1.0);
	}
	// Grass
	else if (normalize(passNormal).y > multiTexYLim) {
		outColor = vec4(shade*vec3(texture(grassTex, passTexCoord)), 1.0);
	}
	// Rocky slope
	else {
		outColor = vec4(shade*vec3(texture(rockTex, passTexCoord)), 1.0);
	}
}
