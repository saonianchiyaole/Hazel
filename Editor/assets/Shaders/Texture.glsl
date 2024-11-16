#type vertex

#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCrood;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in int a_TexIndex;			

uniform mat4 u_Transform;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_TexCrood;		
out vec3 v_Position;
out vec4 v_Color;
flat out int v_TexIndex;

void main()
{		
	v_TexCrood = a_TexCrood;
	v_Position = a_Position;
	v_TexIndex = a_TexIndex;
	v_Color = a_Color;

	gl_Position = u_Projection * u_View * u_Transform *  vec4(a_Position, 1.0);
}


#type fragment

#version 330 core

layout(location = 0) out vec4 color;

uniform sampler2D u_Textures[20];

in vec2 v_TexCrood;
in vec3 v_Position;
in vec4 v_Color;
flat in int v_TexIndex;

void main()
{
	
	switch(v_TexIndex){
		case 0: color = texture(u_Textures[0], v_TexCrood) * v_Color; break;
		case 1: color = texture(u_Textures[1], v_TexCrood) * v_Color; break;
		case 2: color = texture(u_Textures[2], v_TexCrood) * v_Color; break;
		case 3: color = texture(u_Textures[3], v_TexCrood) * v_Color; break;
		case 4: color = texture(u_Textures[4], v_TexCrood) * v_Color; break;
		case 5: color = texture(u_Textures[5], v_TexCrood) * v_Color; break;
		case 6: color = texture(u_Textures[6], v_TexCrood) * v_Color; break;
	}
}