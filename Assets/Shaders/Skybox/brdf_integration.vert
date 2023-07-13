#version 450

layout(std140, binding = 0) uniform LocalConstants
{
	mat4 Matrix;
	mat4 ViewProjection;
	vec4 Eye;
	vec4 Light;
};

layout (location = 0) out vec2 outTexCoord;

void main()
{
	outTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vec4 Position = vec4(outTexCoord * 2.0f + -1.0f, 0.0f, 1.0f);
    Position.y = -Position.y;
    gl_Position = Position;
}