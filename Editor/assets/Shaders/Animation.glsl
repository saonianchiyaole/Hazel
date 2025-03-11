#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;		
layout(location = 1) in vec3 a_Normal;		
layout(location = 2) in vec3 a_Tangent;		
layout(location = 3) in vec3 a_Binormal;		
layout(location = 4) in vec2 a_TexCoord;		
layout(location = 5) in int a_EntityID;		

layout(location = 6) in ivec4 a_BoneIndex;
layout(location = 7) in vec4 a_BoneWeights;

layout(std140, binding = 0) uniform CameraUniform
{
	mat4 u_ViewProjection;
	vec3 cameraPosition;
};

uniform mat4 u_Transform;
uniform mat4 u_View;
uniform mat4 u_Projection;


const int MAX_BONES = 100;
uniform mat4 u_BoneTransforms[100];
uniform bool u_IsAnimation;


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


    mat4 boneTransform = u_BoneTransforms[a_BoneIndices[0]] * a_BoneWeights[0];
    boneTransform += u_BoneTransforms[a_BoneIndices[1]] * a_BoneWeights[1];
    boneTransform += u_BoneTransforms[a_BoneIndices[2]] * a_BoneWeights[2];
    boneTransform += u_BoneTransforms[a_BoneIndices[3]] * a_BoneWeights[3];

    vec4 localPosition = boneTransform * vec4(a_Position, 1.0);

	vs_Output.WorldPosition = vec3(u_Transform * boneTransform * vec4(a_Position, 1.0f));
	vs_Output.Normal = normalize(mat3(u_Transform) * mat3(boneTransform) * a_Normal);
	vs_Output.TexCoord = vec2(a_TexCoord.x, a_TexCoord.y);
	vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
	vs_Output.WorldTransform = mat3(u_Transform) ;
	vs_Output.Binormal = a_Binormal;
	vs_Output.EntityID = a_EntityID;
	vs_Output.cameraPosition = cameraPosition;

	gl_Position = u_ViewProjection * u_Transform * localPosition;
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

uniform sampler2D u_AlbedoTex;
uniform sampler2D u_NormalTex;
uniform sampler2D u_RoughnessTex;
uniform sampler2D u_MetalnessTex;

uniform samplerCube u_EnvIrradiance;
uniform sampler2D u_BRDFLUT;
uniform samplerCube u_EnvRadiance;

uniform vec4 u_Albedo;
uniform float u_Roughness;
uniform float u_Metalness;

//toggle
uniform bool u_UseAlbedoTex;
uniform bool u_UseNormalTex;
uniform bool u_UseRoughnessTex;
uniform bool u_UseMetalnessTex;

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

vec3 fresnelSchlickRoughness(vec3 F0, float NdotH, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - NdotH, 5.0);
} 

vec4 IBL(vec3 F0){
	
	vec3 result = vec3(0.0);

	vec3 L = -light.direction.xyz;
	vec3 N = m_PBRParams.normal;
	vec3 H = normalize(m_PBRParams.view + L);

	float NdotV = m_PBRParams.NdotV;
	float NdotL = max(dot(L, N), 0.0f);
	float NdotH = max(dot(N, H), 0.0f);

	vec3 Lr = 2.0 * m_PBRParams.NdotV * m_PBRParams.normal - m_PBRParams.view;
	Lr.y = -Lr.y;
	//diffuse
	vec3 F = fresnelSchlickRoughness(F0, NdotV, m_PBRParams.roughness);
	vec3 kd = vec3(1.0f) - F;
	kd *= 1.0 - m_PBRParams.metalness;
	result += textureLod(u_EnvIrradiance, m_PBRParams.normal, 0).rgb * m_PBRParams.albedo * kd;

	//specular
	int u_EnvRadianceTexLevels = textureQueryLevels(u_EnvRadiance);
	vec3 specularIrradiance = textureLod(u_EnvRadiance, Lr, (m_PBRParams.roughness) * u_EnvRadianceTexLevels).rgb;
	vec2 specularBRDF = texture(u_BRDFLUT, vec2(m_PBRParams.NdotV,  m_PBRParams.roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

	result += specularIBL;
	return vec4(result, 1.0);

}

vec4 PBRLighting(vec3 F0){

	vec3 L = -light.direction.xyz;
	
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
	vec4 albedoAndTransparency = u_UseAlbedoTex == true ? texture(u_AlbedoTex, vs_Input.TexCoord) : u_Albedo;
	float transparency = albedoAndTransparency.w;

	if(u_UseNormalTex == true){
		m_PBRParams.normal = normalize(2.0 * texture(u_NormalTex, vs_Input.TexCoord).xyz - 1.0f);
		m_PBRParams.normal = normalize(vs_Input.WorldNormals * m_PBRParams.normal);
	}
	else {
		m_PBRParams.normal = vs_Input.Normal;
	}
	m_PBRParams.view = normalize(vs_Input.cameraPosition.xyz - vs_Input.WorldPosition);
	m_PBRParams.NdotV = max(dot(m_PBRParams.normal, m_PBRParams.view), 0.0);
	m_PBRParams.albedo = albedoAndTransparency.xyz;
	m_PBRParams.roughness = u_UseRoughnessTex ? texture(u_RoughnessTex, vs_Input.TexCoord).x : u_Roughness;
	m_PBRParams.metalness = u_UseMetalnessTex ? texture(u_MetalnessTex, vs_Input.TexCoord).x : u_Metalness;

	vec4 ambientIntensity = vec4(0.09f, 0.09f, 0.09f, 0.0f) * vec4(m_PBRParams.albedo, 0.0f);

	vec3 F0 = mix(Fdielectric, m_PBRParams.albedo, m_PBRParams.metalness);
	vec4 lightContribution = PBRLighting(F0); 
	vec4 IBLContribution = IBL(F0);
	//vec4 lightContribution = PhongLighting();

	color = lightContribution + IBLContribution;
	
	IDColor = vs_Input.EntityID;
}