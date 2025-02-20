#type compute
#version 450 core

const float PI = 3.141592;
const float TwoPI = 2 * 3.141592;
const float Epsilon = 0.00001;

const int NumSamples = 1024;
const float InvNumSamples = 1.0 / float(NumSamples);

layout(binding = 0, rgba16f) restrict writeonly uniform image2D outputTexture;


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


vec3 sampleGGX(float u1, float u2, float roughness)
{
	float alpha = roughness * roughness;

	float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha*alpha - 1.0) * u2));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta); // Trig. identity
	float phi = TwoPI * u1;

	// Convert to Cartesian upon return.
	return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
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

//Geometry
float GeometrySchlickGGX(float NdotV, float k){
	return NdotV / ( NdotV * (1.0 - k) + k);
}

float GeometrySmith(float NdotV, float NdotL, float roughness){
	float a = roughness;
	float kDirect = pow(a + 1.0, 2.0) / 8.0;
	float kIBL = a * a / 2.0;
	float ggx1 = GeometrySchlickGGX(NdotV, kDirect);
    float ggx2 = GeometrySchlickGGX(NdotL, kDirect);
	return ggx1 * ggx2;
}



layout(local_size_x = 32, local_size_y = 32) in;
void main(void){

    ivec2 outputSize = imageSize(outputTexture);
    if(gl_GlobalInvocationID.x >= outputSize.x || gl_GlobalInvocationID.y >= outputSize.y) {
		return;
	}
    
    ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 texSize = imageSize(outputTexture);

    // 参数映射 [0,1]
    vec2 uv = vec2(texCoord) / vec2(texSize);
    float NdotV = uv.x;
    float roughness = uv.y;
    

    vec3 V;
    V.x = sqrt(1.0 - NdotV * NdotV);
    V.y = 0.0;
    V.z = NdotV;

    vec3 N = vec3(0.0, 0.0, 1.0);
	vec3 S, T;
	computeBasisVectors(N, S, T);

    float A = 0.0;
    float B = 0.0;
    for(uint i = 0; i < NumSamples; i++){
        vec2 u = sampleHammersley(i);
		vec3 H = tangentToWorld(sampleGGX(u.x, u.y, roughness), N, S, T);
        vec3 L = normalize(2.0 * dot(H, V) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0){
            float G = GeometrySmith(NdotV, NdotL, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }

    }
	A *= InvNumSamples;
	B *= InvNumSamples;

	imageStore(outputTexture, texCoord, vec4(A, B, 0.0, 1.0));

}