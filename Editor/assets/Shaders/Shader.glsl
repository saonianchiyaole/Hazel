#type vertex

#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCrood;			

uniform mat4 u_Transform;
uniform mat4 u_View;
uniform mat4 u_Projection;
//uniform mat4 u_ViewProjection;

out vec2 v_TexCrood;		


void main()
{		
	v_TexCrood = a_TexCrood;
	//gl_Position = u_Transform * u_ViewProjection * vec4(a_Position, 1.0);
	gl_Position = u_Projection * u_View * u_Transform *  vec4(a_Position, 1.0);
}


#type fragment

#version 330 core

layout(location = 0) out vec4 color;

uniform sampler2D u_Texture;		

in vec2 v_TexCrood;

void main()
{
	color = texture(u_Texture, v_TexCrood);
	//color = vec4(v_Position, 1.0f);
}