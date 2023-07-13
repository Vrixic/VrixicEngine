#version 450

layout (location = 0) in vec3 vPos;

layout(std140, binding = 0) uniform LocalConstants
{
	mat4 Matrix;
	mat4 ViewProjection;
	vec4 Eye;
	vec4 Light;
};

layout (location = 0) out vec3 outUVW;

void main()
{
	outUVW = vPos;

	// Convert to homogenous coordinate but do not use translation to compute the new coordinates
	vec4 NewPosition = ViewProjection * vec4(vPos, 0.0);

	// to make sure that the depth value will be 1 in the end for depth testing, override z component with w (perspective divide (z/w) == 1)
	gl_Position = NewPosition.xyww;
}