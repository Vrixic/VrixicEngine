#version 450

layout(std140, binding = 0) uniform LocalConstants
{
	mat4 Matrix;
	mat4 ViewProjection;
	vec4 camPos;
    vec3 Light;
    uint DebugFlags;
};

layout(std140, binding = 1) uniform MaterialConstants
{
    vec4 BaseColorFactor;   ////////
    mat4 Model;             ////////
    mat4 ModelInv;          ////////

    vec3 EmissiveFactor;    ////////
    float    MetallicFactor;     ////////

    float    RoughnessFactor;    ////////
    float    OcclusionFactor;    ////////
    float    AlphaMask;
    float    AlphaMaskCutoff;
             
    uint    Flags;              ////////
    uint    Padding[3];
};

layout (binding = 2) uniform sampler2D colorMap;
layout (binding = 3) uniform sampler2D physicalDescriptorMap;
layout (binding = 4) uniform sampler2D aoMap;
layout (binding = 5) uniform sampler2D emissiveMap;
layout (binding = 6) uniform sampler2D normalMap;

layout (binding = 7) uniform samplerCube samplerIrradiance;
layout (binding = 8) uniform samplerCube prefilteredMap;
layout (binding = 9) uniform sampler2D samplerBRDFLUT;

layout (location = 0) out vec4 outFragColor;

void main(void) 
{
	outFragColor = vec4(1, 1, 0, 1.0); 
}