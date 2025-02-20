#type compute
#version 450 core

const float PI = 3.141592;
const float TwoPI = 2 * PI;
const float Epsilon = 0.00001;

const int NumSamples = 64 * 1024;
const float InvNumSamples = 1.0 / float(NumSamples);

layout(binding = 0) uniform samplerCube inputTexture;
layout(binding = 0, rgba16f) restrict writeonly uniform imageCube outputTexture;


vec3 GetCubeMapTexCoord()
{
    vec2 st = gl_GlobalInvocationID.xy / vec2(imageSize(outputTexture));
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 ret;
    if (gl_GlobalInvocationID.z == 0)      ret = vec3(  1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1) ret = vec3( -1.0, uv.y,  uv.x);
    else if (gl_GlobalInvocationID.z == 2) ret = vec3( uv.x,  1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3) ret = vec3( uv.x, -1.0,  uv.y);
    else if (gl_GlobalInvocationID.z == 4) ret = vec3( uv.x, uv.y,   1.0);
    else if (gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y,  -1.0);
    return normalize(ret);
}

float radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 sampleHammersley(uint i)
{
	return vec2(i * InvNumSamples, radicalInverse_VdC(i));
}


vec3 sampleGGX(float u1, float u2)
{
	// float cosTheta = 1 - u1;
	// float sinTheta = sqrt(1.0 - cosTheta*cosTheta); 
	// float phi = TwoPI * u2;

	// // Convert to Cartesian upon return.
	// return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	const float u1p = sqrt(max(0.0, 1.0 - u1*u1));
	return vec3(cos(TwoPI*u2) * u1p, sin(TwoPI*u2) * u1p, u1);
}

void computeBasisVectors(const vec3 N, out vec3 S, out vec3 T)
{
	// Branchless select non-degenerate T.
	T = cross(N, vec3(0.0, 1.0, 0.0));
	T = mix(cross(N, vec3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));
	
	T = normalize(T);
	S = normalize(cross(N, T));
}

vec3 tangentToWorld(const vec3 v, const vec3 N, const vec3 S, const vec3 T)
{
	return S * v.x + T * v.y + N * v.z;
}





layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main(void){

    ivec2 outputSize = imageSize(outputTexture);
    if(gl_GlobalInvocationID.x >= outputSize.x || gl_GlobalInvocationID.y >= outputSize.y) {
		return;
	}
    
    vec3 N = GetCubeMapTexCoord();
	vec3 S, T;
	computeBasisVectors(N, S, T);

	vec3 irradiance = vec3(0);
    for(uint i = 0; i < NumSamples; i++){
        vec2 u = sampleHammersley(i);
		vec3 Li = tangentToWorld(sampleGGX(u.x, u.y), N, S, T);

		float dotLi = max(dot(Li, N), 0.0);

		irradiance += 2.0 * textureLod(inputTexture, Li, 0).rgb * dotLi;

    }
	irradiance *= InvNumSamples;

	imageStore(outputTexture, ivec3(gl_GlobalInvocationID), vec4(irradiance, 1.0));

}