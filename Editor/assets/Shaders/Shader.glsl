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
	vec4 cameraPosition;
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
	flat vec4 cameraPosition;
}vs_Output;



void main()
{		
	vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0f));
	vs_Output.Normal = mat3(u_Transform) * a_Normal;
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
	flat vec4 cameraPosition;
}vs_Input;



uniform sampler2D u_Albedo;
uniform sampler2D u_Normal;

uniform float ambientFactor;
uniform float diffuseFactor;
uniform float specFactor;

vec4 PhongLighting(){
	
	
	vec3 view = normalize(vs_Input.cameraPosition.xyz - vs_Input.WorldPosition);
	vec3 lightDir = normalize(-light.direction.xyz);
	
	vec4 albedo = texture(u_Albedo, vs_Input.TexCoord);
	float transparency = albedo.w; 

	float ambient = ambientFactor;

	float diffuse = diffuseFactor * max(dot(vs_Input.Normal, lightDir), 0.0f);
	
	vec3 halfNormal = normalize(lightDir + view);
	float specular = specFactor * pow(max(dot(halfNormal, vs_Input.Normal), 0), 32);


	//return vec4((ambient + diffuse + specular) * light.color * vec3(albedo), transparency);

	return vec4((ambient + diffuse + specular) * light.color.xyz * albedo.xyz, 1.0f);
}


vec4 PBRLighting(){
	return vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

void main()
{

	vec4 lightContribution = PhongLighting(); 

	color = lightContribution;
	IDColor = vs_Input.EntityID;
}