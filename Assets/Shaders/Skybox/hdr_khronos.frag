#version 450

layout(std140, binding = 0) uniform HDRBuffer
{
	int CubemapFace;
};

layout (binding = 1) uniform sampler2D equirectangularMap;

layout (location = 0) in vec2 inTexCoord;

layout (location = 0) out vec4 outFragColor;

#define MATH_PI 3.1415926535897932384626433832795
#define MATH_INV_PI (1.0 / MATH_PI)

//const vec2 INV_ATAN = vec2(0.1591, 0.3183);
//vec2 SampleSphericalMap(in vec3 v)
//{
//    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
//    uv *= INV_ATAN;
//    uv += 0.5;
//    return uv;
//}

vec3 uvToXYZ(int face, vec2 uv)
{
	if(face == 0)
		return vec3(     1.f,   uv.y,    -uv.x);

	else if(face == 1)
		return vec3(    -1.f,   uv.y,     uv.x);

	else if(face == 2)
		return vec3(   +uv.x,   -1.f,    +uv.y);

	else if(face == 3)
		return vec3(   +uv.x,    1.f,    -uv.y);

	else if(face == 4)
		return vec3(   +uv.x,   uv.y,      1.f);

	else //if(face == 5)
	{	return vec3(    -uv.x,  +uv.y,     -1.f);}
}

vec2 dirToUV(vec3 dir)
{
	return vec2(
		0.5f + 0.5f * atan(dir.z, dir.x) / MATH_PI,
		1.f - acos(dir.y) / MATH_PI);
}

vec3 panoramaToCubeMap(int face, vec2 texCoord)
{
	vec2 texCoordNew = texCoord*2.0-1.0;
	vec3 scan = uvToXYZ(face, texCoordNew);
	vec3 direction = normalize(scan);
	vec2 src = dirToUV(direction);

	return  texture(equirectangularMap, src).rgb;
}

void main(void)
{
    outFragColor = vec4(1.0, 0.0, 0.0, 1.0);
	outFragColor.rgb = panoramaToCubeMap(CubemapFace, inTexCoord);
}