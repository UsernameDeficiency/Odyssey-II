#version 400
in vec3 inPos;

//uniform mat4 worldToView;
//uniform mat4 projection;

void main()
{
    gl_Position = vec4(inPos, 1.0);
}

//out vec2 uv;
//
//void main()
//{
//	float x = float(((uint(gl_VertexID) + 2u) / 3u)%2u);
//	float y = float(((uint(gl_VertexID) + 1u) / 3u)%2u);
//
//	gl_Position = vec4(-1.0f + x * 2.0f, -1.0f + y * 2.0f, 1.0f, 1.0f);
//	uv = vec2(x, y);
//}
