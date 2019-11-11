#version 400
out vec4 outColor;

void main()
{
	float zNear = 3.0f;
	float zFar = 18000.0f/3.0f;
	float z = gl_FragCoord.z * 2.0 - 1.0; // back to NDC
	float depth = (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
	vec4 fog = vec4(0.5 + 0.5*vec3(1 - depth/zFar), 0.5);
	outColor = vec4(0.5, 0.5, 0.5, 0.5*(1 - depth/zFar));
}
