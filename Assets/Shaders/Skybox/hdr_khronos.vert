#version 450

layout (location = 0) in vec3 vPos;
layout (location = 0) out vec2 outTexCoord;

precision highp float;

void main(void) 
{
    outTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vec4 Position = vec4(outTexCoord * 2.0f + -1.0f, 0.0f, 1.0f);
    Position.y = -Position.y;
    gl_Position = Position;
}