#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;		
layout(location = 1) in vec3 a_Normal;		
layout(location = 2) in vec3 a_Tangent;		
layout(location = 3) in vec3 a_Binormal;		
layout(location = 4) in vec2 a_TexCoord;		
layout(location = 5) in int a_EntityID;		


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
	int EntityID;
}vs_Output;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

void main()
{		
	vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0f));
	vs_Output.Normal = mat3(u_Transform) * a_Position;
	vs_Output.TexCoord = vec2(a_TexCoord.x, a_TexCoord.y);
	vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
	vs_Output.WorldTransform = mat3(u_Transform);
	vs_Output.Binormal = a_Binormal;
	vs_Output.EntityID = a_EntityID;

	gl_Position = u_ViewProjection * u_Transform *  vec4(a_Position, 1.0);
}


#type fragment

#version 450 core

layout(location = 0) out vec4 color;	
layout(location = 1) out int IDColor;

in VertexOutput{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
	int EntityID;
}vs_Input;


uniform sampler2D u_Albedo;

void main()
{
	color = texture(u_Albedo, vs_Input.TexCoord);
	IDColor = vs_Input.EntityID;
}