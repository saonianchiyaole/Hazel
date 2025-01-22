#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;		
layout(location = 1) in vec3 a_Normal;		
layout(location = 2) in vec3 a_Tangent;		
layout(location = 3) in vec3 a_Binormal;		
layout(location = 4) in vec2 a_TexCoord;		
layout(location = 5) in int a_EntityID;		


layout(std140, binding = 0) uniform CameraUniform
{
	mat4 u_ViewProjection;
	vec3 cameraPosition;
};

uniform mat4 u_Transform;
uniform mat4 u_View;
uniform mat4 u_Projection;


out VertexOutput{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
	flat int EntityID;
	flat vec3 cameraPosition;
}vs_Output;



void main()
{		
	vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0f));
	vs_Output.Normal = normalize(mat3(u_Transform) * a_Normal);
	vs_Output.TexCoord = vec2(a_TexCoord.x, a_TexCoord.y);
	vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
	vs_Output.WorldTransform = mat3(u_Transform);
	vs_Output.Binormal = a_Binormal;
	vs_Output.EntityID = a_EntityID;
	vs_Output.cameraPosition = cameraPosition;

	gl_Position = u_ViewProjection * u_Transform *  vec4(a_Position, 1.0);
}


#type fragment

#version 450 core

layout(location = 0) out vec4 color;	
layout(location = 1) out int IDColor;



layout(std140, binding = 1) uniform LightUniform
{
	struct {	
		vec4 position;
		vec4 direction;
		vec4 color;
	}light;
};


in VertexOutput{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
	flat int EntityID;
	flat vec3 cameraPosition;
}vs_Input;


const float PI = 3.141592;
const float Epsilon = 0.00001;
const vec3 Fdielectric = vec3(0.04f);

struct PBRParameters{
	vec3 albedo;
	float roughness;
	float metalness;

	vec3 normal;
	vec3 view;
	float NdotV;
};

PBRParameters m_PBRParams;

uniform sampler2D u_Albedo;
uniform sampler2D u_Normal;
uniform sampler2D u_Roughness;
uniform sampler2D u_Metalness;

uniform float ambientFactor;
uniform float diffuseFactor;
uniform float specFactor;

// ---------------------------------------------------------------------------------------------------
// The following code (from Unreal Engine 4's paper) shows how to filter the environment map
// for different roughnesses. This is mean to be computed offline and stored in cube map mips,
// so turning this on online will cause poor performance
// float RadicalInverse_VdC(uint bits) 
// {
//     bits = (bits << 16u) | (bits >> 16u);
//     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
//     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
//     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
//     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
//     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
// }

// vec2 Hammersley(uint i, uint N)
// {
//     return vec2(float(i)/float(N), RadicalInverse_VdC(i));
// }

// vec3 ImportanceSampleGGX(vec2 Xi, float Roughness, vec3 N)
// {
// 	float a = Roughness * Roughness;
// 	float Phi = 2 * PI * Xi.x;
// 	float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
// 	float SinTheta = sqrt( 1 - CosTheta * CosTheta );
// 	vec3 H;
// 	H.x = SinTheta * cos( Phi );
// 	H.y = SinTheta * sin( Phi );
// 	H.z = CosTheta;
// 	vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
// 	vec3 TangentX = normalize( cross( UpVector, N ) );
// 	vec3 TangentY = cross( N, TangentX );
// 	// Tangent to world space
// 	return TangentX * H.x + TangentY * H.y + N * H.z;
// }

// float TotalWeight = 0.0;

// vec3 PrefilterEnvMap(float Roughness, vec3 R)
// {
// 	vec3 N = R;
// 	vec3 V = R;
// 	vec3 PrefilteredColor = vec3(0.0);
// 	int NumSamples = 1024;
// 	for(int i = 0; i < NumSamples; i++)
// 	{
// 		vec2 Xi = Hammersley(i, NumSamples);
// 		vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
// 		vec3 L = 2 * dot(V, H) * H - V;
// 		float NoL = clamp(dot(N, L), 0.0, 1.0);
// 		if (NoL > 0)
// 		{
// 			PrefilteredColor += texture(u_EnvRadianceTex, L).rgb * NoL;
// 			TotalWeight += NoL;
// 		}
// 	}
// 	return PrefilteredColor / TotalWeight;
// }

// ---------------------------------------------------------------------------------------------------




vec4 PhongLighting(){
	
	
	vec3 view = normalize(vs_Input.cameraPosition.xyz - vs_Input.WorldPosition);
	vec3 lightDir = normalize(-light.direction.xyz);
	
	vec3 normal = texture(u_Normal, vs_Input.TexCoord).xyz;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(vs_Input.WorldNormals * normal);


	vec4 albedo = texture(u_Albedo, vs_Input.TexCoord);
	float transparency = albedo.w; 

	

	float ambient = ambientFactor;

	float diffuse = diffuseFactor * max(dot(normal, lightDir), 0.0f);
	
	vec3 halfNormal = normalize(lightDir + view);
	float specular = specFactor * pow(max(dot(halfNormal, normal), 0), 32);


	return vec4((ambient + diffuse + specular) * light.color.xyz * albedo.xyz, transparency);
	//return m_PBRParams.roughness
}


void GetNormalFormNormalMap(vec2 coord){

}

//Geometry
float GeometrySchlickGGX(float NdotV, float k){
	return NdotV / ( NdotV * (1 - k) + k);
}

float GeometrySmith(float NdotV, float NdotL){
	float a = m_PBRParams.roughness;
	float kDirect = pow(a + 1, 2) / 8;
	float kIBL = a * a / 2;
	float ggx1 = GeometrySchlickGGX(NdotV, kDirect);
    float ggx2 = GeometrySchlickGGX(NdotL, kDirect);
	return ggx1 * ggx2;
}

// Normal Distribution Function , NDF
float TrowbridgeReitzGGX(float NdotH){
	float NdotH2 = NdotH * NdotH;
	float a2 = pow(m_PBRParams.roughness, 2);
	float denominator = (NdotH2 * (a2 - 1) + 1);
	denominator =  PI * pow(denominator, 2);
	return a2 / denominator;
}

// Fresnel
vec3 FresnelSchlick(vec3 F0, float NdotH){
	return F0 + (1 - F0) * pow(1 - NdotH, 5);
}


vec4 PBRLighting(){

	vec3 L = -light.direction.xyz;
	vec3 F0 = mix(Fdielectric, m_PBRParams.albedo, m_PBRParams.metalness);
	vec3 N = m_PBRParams.normal;
	vec3 H = normalize(m_PBRParams.view + L);
	
	float NdotV = m_PBRParams.NdotV;
	float NdotL = max(dot(L, N), 0.0f);
	float NdotH = max(dot(N, H), 0.0f);
	//specular
	
	vec3 F = FresnelSchlick(F0, NdotH); //Fresnel
	float NDF = TrowbridgeReitzGGX(NdotH);
	float G = GeometrySmith(NdotV, NdotL); 

	//diffuse
	vec3 ks = F;
	vec3 kd = vec3(1.0f) - ks;
	kd *= 1.0 - m_PBRParams.metalness;
	vec3 fLambert = m_PBRParams.albedo / PI;
	vec3 diffuse = kd * fLambert;
	
	// combine 
	vec3 nominator = ks * NDF * F * G;
	float denominator = 4.0f * NdotV * NdotL + 0.0001;
	vec3 specular = nominator / denominator;

	vec3 result = (diffuse + specular) * light.color.xyz * NdotL;
	return vec4(result, 1.0);
}

void main()
{
	vec4 albedoAndTransparency = texture(u_Albedo, vs_Input.TexCoord);
	float transparency = albedoAndTransparency.w;
	m_PBRParams.normal = normalize(2.0 * texture(u_Normal, vs_Input.TexCoord).xyz - 1.0f);
	m_PBRParams.view = normalize(vs_Input.cameraPosition.xyz - vs_Input.WorldPosition);
	m_PBRParams.NdotV = max(dot(m_PBRParams.normal, m_PBRParams.view), 0.0);
	m_PBRParams.albedo = albedoAndTransparency.xyz;
	m_PBRParams.roughness = texture(u_Roughness, vs_Input.TexCoord).x;
	m_PBRParams.metalness = texture(u_Metalness, vs_Input.TexCoord).x;

	vec4 ambientIntensity = vec4(0.09f, 0.09f, 0.09f, 0.0f) * vec4(m_PBRParams.albedo, 0.0f);

	vec4 lightContribution = PBRLighting(); 
	//vec4 lightContribution = PhongLighting();

	color = lightContribution + ambientIntensity;

	IDColor = vs_Input.EntityID;
}