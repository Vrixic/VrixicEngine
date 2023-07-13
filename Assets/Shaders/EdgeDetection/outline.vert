#version 450

layout(std140, binding = 0) uniform LocalConstants
{
	mat4 Matrix;
	mat4 ViewProjection;
	vec4 Eye;
	vec4 Light;
};

layout(std140, binding = 1) uniform MaterialConstants 
{
    vec4 BaseColorFactor;   ////////
    mat4 ModelMatrix;             ////////
    mat4 ModelInverse;          ////////

    vec3 EmissiveFactor;    ////////
    float    MetallicFactor;     ////////

    float    RoughnessFactor;    ////////
    float    OcclusionFactor;    ////////
    float    AlphaMask;
    float    AlphaMaskCutoff;
             
    uint    Flags;              ////////
    uint    Padding[3];
};

layout(location=0) in vec3 Position;
layout(location=1) in vec4 Tangent;
layout(location=2) in vec3 Normal;
layout(location=3) in vec2 TexCoord0;

precision highp float;

void main(void) 
{
	// Extrude along normal
	vec4 pos = vec4(Position.xyz + Normal * 0.025f, 1);
    vec4 HomogenousCoords = (ViewProjection * ModelMatrix * pos).xyzz; // basically we want the to be written be higher than it would be written in a original pass
    HomogenousCoords.z -= 0.025f; // make the vertex appear infront of original vertex 
	gl_Position = HomogenousCoords;
}