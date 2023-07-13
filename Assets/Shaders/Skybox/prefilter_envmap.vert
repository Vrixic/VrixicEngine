#version 450

layout (location = 0) in vec3 vPos;
layout (location = 0) out vec3 outWorldPos;

layout(std140, binding = 0) uniform LocalConstants
{
	mat4 Matrix;
	mat4 ViewProjection;
	vec4 Eye;
	vec4 Light;
};

void main(void) 
{
    outWorldPos = vPos;  
    gl_Position =  ViewProjection * vec4(outWorldPos, 1.0);
}