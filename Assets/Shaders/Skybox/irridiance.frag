#version 450

layout (binding = 1) uniform samplerCube environmentMap;

layout (location = 0) in vec3 inWorldPos;

layout (location = 0) out vec4 outFragColor;

#define PI 3.1415926535897932384626433832795

void main()
{		
	// The world vector acts as the normal of a tangent surface
    // from the origin, aligned to WorldPos. Given this normal, calculate all
    // incoming radiance of the environment. The result of this radiance
    // is the radiance of light coming from -Normal direction, which is what
    // we use in the PBR shader to sample irradiance.
    vec3 N = normalize(inWorldPos);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));

    const float TWO_PI = PI * 2.0;
	const float HALF_PI = PI * 0.5;
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < TWO_PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < HALF_PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = (PI * irradiance) / float(nrSamples);
    
    outFragColor = vec4(irradiance, 1.0);
}