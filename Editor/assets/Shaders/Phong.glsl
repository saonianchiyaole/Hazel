#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;		
layout(location = 1) in vec3 a_Normal;		
layout(location = 2) in vec3 a_Tangent;		
layout(location = 3) in vec3 a_Binormal;		
layout(location = 4) in vec2 a_TexCoord;		

layout(location = 5) in ivec4 a_BoneIndices;
layout(location = 6) in vec4 a_BoneWeights;

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
	flat vec3 cameraPosition;
}vs_Output;



void main()
{		
	vec4 localPosition = vec4(a_Position, 1.0);
	if(u_IsAnimation){
		mat4 boneTransform = u_BoneTransforms[a_BoneIndices[0]] * a_BoneWeights[0];
    	boneTransform += u_BoneTransforms[a_BoneIndices[1]] * a_BoneWeights[1];
    	boneTransform += u_BoneTransforms[a_BoneIndices[2]] * a_BoneWeights[2];
    	boneTransform += u_BoneTransforms[a_BoneIndices[3]] * a_BoneWeights[3];
		localPosition = boneTransform * localPosition;
		
		vs_Output.WorldPosition = vec3(u_Transform * boneTransform * vec4(a_Position, 1.0f));
		vs_Output.Normal = normalize(mat3(u_Transform) * mat3(boneTransform) * a_Normal);
		//vs_Output.WorldPosition = vec3(u_Transform * boneTransform * vec4(a_Position, 1.0f));
		//vs_Output.Normal = normalize(mat3(u_Transform) * boneTransform * a_Normal);
	}
    else{
		vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0f));
		vs_Output.Normal = normalize(mat3(u_Transform) * a_Normal);
	}
	
	//vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0f));
	//vs_Output.Normal = normalize(mat3(u_Transform) * a_Normal);
	vs_Output.TexCoord = vec2(a_TexCoord.x, a_TexCoord.y);
	vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
	vs_Output.WorldTransform = mat3(u_Transform) ;
	vs_Output.Binormal = a_Binormal;
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
	flat vec3 cameraPosition;
}vs_Input;


const float PI = 3.141592;
const float Epsilon = 0.00001;

uniform sampler2D u_AlbedoTex;
uniform sampler2D u_NormalTex;

//toggle
uniform bool u_UseAlbedoTex;
uniform bool u_UseNormalTex;

uniform int u_EntityID;

uniform vec4 u_Albedo;
uniform float ambientFactor;
uniform float diffuseFactor;
uniform float specFactor;

vec4 PhongLighting(){
	
	
	vec3 view = normalize(vs_Input.cameraPosition.xyz - vs_Input.WorldPosition);
	vec3 lightDir = normalize(-light.direction.xyz);
	
	vec3 normal = vs_Input.Normal;
	//texture(u_NormalTex, vs_Input.TexCoord).xyz;
	//normal = normalize(normal * 2.0 - 1.0);
	//normal = normalize(vs_Input.WorldNormals * normal);


	vec4 albedo = u_Albedo;
	//texture(u_AlbedoTex, vs_Input.TexCoord);
	float transparency = albedo.w; 

	

	float ambient = ambientFactor;

	float diffuse = diffuseFactor * max(dot(normal, lightDir), 0.0f);
	
	vec3 halfNormal = normalize(lightDir + view);
	float specular = specFactor * pow(max(dot(halfNormal, normal), 0), 32);


	return vec4((ambient + diffuse + specular) * light.color.xyz * albedo.xyz, transparency);
	//return m_PBRParams.roughness
}


void main()
{
	
	//vec4 lightContribution = PBRLighting(); 
	vec4 lightContribution = PhongLighting();

	color = lightContribution;

	IDColor = u_EntityID;
}