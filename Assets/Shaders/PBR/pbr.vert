#version 450

uint MaterialFeatures_ColorTexture     = 1 << 0;
uint MaterialFeatures_NormalTexture    = 1 << 1;
uint MaterialFeatures_RoughnessTexture = 1 << 2;
uint MaterialFeatures_OcclusionTexture = 1 << 3;
uint MaterialFeatures_EmissiveTexture =  1 << 4;
uint MaterialFeatures_TangentVertexAttribute = 1 << 5;
uint MaterialFeatures_TexcoordVertexAttribute = 1 << 6;

layout(std140, binding = 0) uniform LocalConstants
{
	mat4 Matrix;
	mat4 ViewProjection;
	vec4 Eye;
	vec4 Light;
};

//layout(std140, binding = 1) uniform MaterialConstants {
//    vec4 BaseColorFactor;
//    mat4 ModelMatrix;
//    mat4 ModelInverse;
//
//    vec3  EmissiveFactor;
//    float MetallicFactor;
//
//    float RoughnessFactor;
//    float OcclusionFactor;
//    uint  Flags;
//};

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

layout (location = 0) out vec2 vTexcoord0;
layout (location = 1) out vec3 vNormal;
layout (location = 2) out vec4 vTangent;
layout (location = 3) out vec3 vPosition;

void main() {
    vPosition = vec3(Matrix * ModelMatrix * vec4(Position, 1.0));
    gl_Position = ViewProjection * vec4(vPosition, 1.0);

    if ( ( Flags & MaterialFeatures_TexcoordVertexAttribute ) != 0 ) {
        vTexcoord0 = TexCoord0;
    }
    vNormal = (ModelInverse * vec4(Normal, 0.0)).xyz;

    if ( ( Flags & MaterialFeatures_TangentVertexAttribute ) != 0 ) {
        vTangent = vec4(mat3(ModelMatrix) * Tangent.xyz, Tangent.w);
    }
}