#version 450

uint MaterialFeatures_ColorTexture     = 1 << 0;
uint MaterialFeatures_NormalTexture    = 1 << 1;
uint MaterialFeatures_RoughnessTexture = 1 << 2;
uint MaterialFeatures_OcclusionTexture = 1 << 3;
uint MaterialFeatures_EmissiveTexture =  1 << 4;
uint MaterialFeatures_TangentVertexAttribute = 1 << 5;
uint MaterialFeatures_TexcoordVertexAttribute = 1 << 6;

uint DebugFlags_DisableSRGBConversion = 1 << 0;
uint DebugFlags_OnlyDiffuseContribution = 1 << 1;
uint DebugFlags_OnlyDiffuseLightContribution = 1 << 2;
uint DebugFlags_OnlySpecularContribution = 1 << 3;
uint DebugFlags_OnlySpecularLightContribution = 1 << 4;
uint DebugFlags_OnlyLightContribution = 1 << 5;

layout(std140, binding = 0) uniform LocalConstants
{
	mat4 Matrix;
	mat4 ViewProjection;
	vec4 camPos;
    vec3 Light;
    uint DebugFlags;

    vec3 LightPositions[4];
	vec3 LightColors[4];
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

layout (location = 0) in vec2 vTexcoord0;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inTangent;
layout (location = 3) in vec3 inWorldPos;

layout (location = 0) out vec4 outColor;

const float M_PI = 3.141592653589793;
const float INV_PI = (1 / M_PI);
const float c_MinRoughness = 0.04;

const float EXPOSURE = 1.0;
const float GAMMA = 1.0;

const float PrefilteredCubeMipLevels = 4.0;

#define MANUAL_SRGB 1

#define CHECK_DEBUG_FLAG(flag) (DebugFlags & flag) != 0

vec3 Uncharted2Tonemap(vec3 color)
{
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;
	const float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec4 tonemap(vec4 color)
{
	vec3 outcol = Uncharted2Tonemap(color.rgb * EXPOSURE);
	outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	return vec4(pow(outcol, vec3(1.0f / GAMMA)), color.a);
}

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
	if(!(CHECK_DEBUG_FLAG(DebugFlags_DisableSRGBConversion)))
	{
		return srgbIn;
	}

	#ifdef MANUAL_SRGB
	#ifdef SRGB_FAST_APPROXIMATION
	vec3 linOut = pow(srgbIn.xyz,vec3(2.2));
	#else //SRGB_FAST_APPROXIMATION
	vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
	vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	#endif //SRGB_FAST_APPROXIMATION
	return vec4(linOut,srgbIn.w);;
	#else //MANUAL_SRGB
	return srgbIn;
	#endif //MANUAL_SRGB
}

vec3 CalculateNormal()
{
     mat3 TBN = mat3( 1.0 );

	 vec3 Normal = normalize(inNormal);

    if ( ( Flags & MaterialFeatures_TangentVertexAttribute ) != 0 ) {
        vec3 tangent = normalize( inTangent.xyz );
        vec3 bitangent = cross( Normal, tangent ) * inTangent.w;

        TBN = mat3(
            tangent,
            bitangent,
            Normal
        );
    }
    else {
        // NOTE(marco): taken from https://community.khronos.org/t/computing-the-tangent-space-in-the-fragment-shader/52861
        vec3 Q1 = dFdx( inWorldPos );
        vec3 Q2 = dFdy( inWorldPos );
        vec2 st1 = dFdx( vTexcoord0 );
        vec2 st2 = dFdy( vTexcoord0 );

        vec3 T = normalize(  Q1 * st2.t - Q2 * st1.t );
        vec3 B = normalize( -Q1 * st2.s + Q2 * st1.s );

        // the transpose of texture-to-eye space matrix
        TBN = mat3(
            T,
            B,
            Normal
        );
    }

    // NOTE(marco): normal textures are encoded to [0, 1] but need to be mapped to [-1, 1] value
    vec3 N = Normal;
    if ( ( Flags & MaterialFeatures_NormalTexture ) != 0 ) {
    
        N = normalize( texture(normalMap, vTexcoord0).rgb * 2.0 - 1.0 );
        N = normalize( TBN * N );
    }

    return N;
}

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
vec3 diffuse(in vec3 inDiffuseColor)
{
	return inDiffuseColor * INV_PI;
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(in float inNdotL, in float inNdotV, in float inAlphaRoughness)
{
	float NdotL = inNdotL;
	float NdotV = inNdotV;
	float r = inAlphaRoughness;

	float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
	float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
	return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(in float inNdotH, in float inAlphaRoughness)
{
	float roughnessSq = inAlphaRoughness * inAlphaRoughness;
	float f = (inNdotH * roughnessSq - inNdotH) * inNdotH + 1.0;
	return roughnessSq / (M_PI * f * f);
}

// Gets metallic factor from specular glossiness workflow inputs 
float convertMetallic(vec3 diffuse, vec3 specular, float maxSpecular) {
	float perceivedDiffuse = sqrt(0.299 * diffuse.r * diffuse.r + 0.587 * diffuse.g * diffuse.g + 0.114 * diffuse.b * diffuse.b);
	float perceivedSpecular = sqrt(0.299 * specular.r * specular.r + 0.587 * specular.g * specular.g + 0.114 * specular.b * specular.b);
	if (perceivedSpecular < c_MinRoughness) {
		return 0.0;
	}
	float a = c_MinRoughness;
	float b = perceivedDiffuse * (1.0 - maxSpecular) / (1.0 - c_MinRoughness) + perceivedSpecular - 2.0 * c_MinRoughness;
	float c = c_MinRoughness - perceivedSpecular;
	float D = max(b * b - 4.0 * a * c, 0.0);
	return clamp((-b + sqrt(D)) / (2.0 * a), 0.0, 1.0);
}

vec3 FresnelSchlickRoughness(in float inCosTheta, in vec3 inF0, in float inRoughness)
{
    return inF0 + (max(vec3(1.0 - inRoughness), inF0) - inF0) * pow(clamp(1.0 - inCosTheta, 0.0, 1.0), 5.0);
}  

void main()
{
	float perceptualRoughness;
	float metallic;
	vec3 diffuseColor;
	vec4 baseColor;

	vec3 f0 = vec3(0.04);

	if (AlphaMask == 1.0f) {
		if (( Flags & MaterialFeatures_ColorTexture ) != 0 ) {
			baseColor = SRGBtoLINEAR(texture(colorMap, vTexcoord0)) * BaseColorFactor;
		} else {
			baseColor = BaseColorFactor;
		}

		if (baseColor.a < AlphaMaskCutoff) {
			discard;
		}
	}

	// Metallic and Roughness material properties are packed together
	// In glTF, these factors can be specified by fixed scalar values
	// or from a metallic-roughness map
	perceptualRoughness = RoughnessFactor; // roughness value, as authored by the model creator (input to shader)
	metallic = MetallicFactor;
	if (( Flags & MaterialFeatures_RoughnessTexture ) != 0 ) {
		// Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
		// This layout intentionally reserves the 'r' channel for (optional) occlusion map data
		vec4 mrSample = texture(physicalDescriptorMap, vTexcoord0);
		perceptualRoughness *= mrSample.g;
		metallic *= mrSample.b;
	} 
	else {
		perceptualRoughness = clamp(perceptualRoughness, c_MinRoughness, 1.0);
		metallic = clamp(metallic, 0.0, 1.0);
	}
	// Roughness is authored as perceptual roughness; as is convention,
	// convert to material roughness by squaring the perceptual roughness [2].
	
	// The albedo may be defined from a base texture or a flat color
	if (( ( Flags & MaterialFeatures_ColorTexture ) != 0 )) {
		baseColor = SRGBtoLINEAR(texture(colorMap, vTexcoord0)) * BaseColorFactor;
	} 
	else {
		baseColor = BaseColorFactor;
	}

	diffuseColor = baseColor.rgb * (vec3(1.0) - f0); // color contribution from diffuse lighting
	diffuseColor *= 1.0 - metallic;

	vec3 n = CalculateNormal();
	vec3 v = normalize(camPos.xyz - inWorldPos);    // Vector from surface point to camera
	vec3 reflection = -normalize(reflect(v, n));
	//reflection.y *= -1.0f; // invert y 
	float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0); // cos angle between normal and view direction
		
	float alphaRoughness = perceptualRoughness * perceptualRoughness; // roughness mapped to a more linear change in the roughness (proposed by [2])

	vec3 specularColor = mix(f0, baseColor.rgb, metallic); // color contribution from specular lighting

	// Compute reflectance.
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b); // full reflectance color (normal incidence angle)
	// For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
	// For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0); // reflectance color at grazing angle
	vec3 specularEnvironmentR0 = specularColor.rgb;
	vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;
	
	// reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
		vec3 ToLight = LightPositions[i] - inWorldPos;
		vec3 l = normalize(ToLight);     // Vector from surface point to light
		vec3 h = normalize(l+v);                        // Half vector between both l and v
		
        float distance = length(ToLight);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = LightColors[i] * attenuation;

		float NdotL = clamp(dot(n, l), 0.001, 1.0); // cos angle between normal and light direction
		float NdotH = clamp(dot(n, h), 0.0, 1.0); // cos angle between normal and half vector
		float LdotH = clamp(dot(l, h), 0.0, 1.0); // cos angle between light direction and half vector
		float VdotH = clamp(dot(v, h), 0.0, 1.0); // cos angle between view direction and half vector
		
		// Calculate the shading terms for the microfacet specular shading model
		vec3 F =  specularEnvironmentR0 + ( specularEnvironmentR90 - specularEnvironmentR0 ) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
		float G = geometricOcclusion(NdotL, NdotV, alphaRoughness);
		float D = microfacetDistribution(NdotH, alphaRoughness);
		
		// Calculation of analytical lighting contribution
		vec3 diffuseContrib = (1.0 - F) * diffuse(diffuseColor);
		vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
		// Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
		Lo += NdotL * radiance * (diffuseContrib + specContrib);
	}

	// Calculate lighting contribution from image based lighting source (IBL)
    vec3 diffuseLight = texture(samplerIrradiance, n).rgb;
    vec3 diffuse      = diffuseLight * diffuseColor;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    vec3 specularLight = textureLod(prefilteredMap, reflection,  perceptualRoughness * PrefilteredCubeMipLevels).rgb;    
    vec2 brdf  = texture(samplerBRDFLUT, vec2(NdotV, perceptualRoughness)).rg;
    vec3 specular = specularLight * (specularColor * brdf.x + brdf.y);

	vec3 color = Lo + diffuse + specular;

	// Apply optional PBR terms for additional (optional) shading
	if (( Flags & MaterialFeatures_OcclusionTexture ) != 0 ) {
		float ao = texture(aoMap, vTexcoord0).r;
		color = mix(color, color * ao, OcclusionFactor);
	}

	const float u_EmissiveFactor = 1.0f;
	if (( Flags & MaterialFeatures_EmissiveTexture ) != 0 ) {
		vec3 emissive = SRGBtoLINEAR(texture(emissiveMap, vTexcoord0)).rgb * EmissiveFactor;
		color += emissive;
	}
	
	outColor = vec4(color, baseColor.a);

	if(CHECK_DEBUG_FLAG(DebugFlags_OnlyDiffuseContribution))
	{
		outColor = vec4(diffuse, baseColor.a);
	}
	else if(CHECK_DEBUG_FLAG(DebugFlags_OnlySpecularContribution))
	{
		outColor = vec4(specular, baseColor.a);
	}
	else if(CHECK_DEBUG_FLAG(DebugFlags_OnlyLightContribution))
	{
		outColor = vec4(Lo, baseColor.a);
	}
	else if(CHECK_DEBUG_FLAG(DebugFlags_OnlySpecularLightContribution))
	{
		outColor = vec4(specularLight, baseColor.a);
	}
	else if(CHECK_DEBUG_FLAG(DebugFlags_OnlyDiffuseLightContribution))
	{
		outColor = vec4(diffuseLight, baseColor.a);
	}
}