#type vertex

#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCrood;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in int a_TexIndex;		

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout(location = 0) out vec2 v_TexCrood;		
layout(location = 1) out vec3 v_Position;
layout(location = 2) out vec4 v_Color;
layout(location = 3) flat out int v_TexIndex;

void main()
{		
	v_TexCrood = a_TexCrood;
	v_Position = a_Position;
	v_TexIndex = a_TexIndex;
	v_Color = a_Color;	
	
	gl_Position = u_ViewProjection *  vec4(a_Position, 1.0);
}


#type fragment

#version 450 core

layout(location = 0) out vec4 color;

layout(binding = 1) uniform sampler2D u_Textures[20];

layout(location = 0) in vec2 v_TexCrood;
layout(location = 1) in vec3 v_Position;
layout(location = 2) in vec4 v_Color;
layout(location = 3) flat in int v_TexIndex;


void main()
{

	vec4 textureColor = vec4(0.0, 0.0, 0.0, 0.0);
	switch(v_TexIndex)
	{
		case 0: textureColor = texture(u_Textures[0], v_TexCrood); break;
		case 1: textureColor = texture(u_Textures[1], v_TexCrood); break;
		case 2: textureColor = texture(u_Textures[2], v_TexCrood); break;
		case 3: textureColor = texture(u_Textures[3], v_TexCrood); break;
		case 4: textureColor = texture(u_Textures[4], v_TexCrood); break;
		case 5: textureColor = texture(u_Textures[5], v_TexCrood); break;
		case 6: textureColor = texture(u_Textures[6], v_TexCrood); break;
		case 7: textureColor = texture(u_Textures[7], v_TexCrood); break;
		case 8: textureColor = texture(u_Textures[8], v_TexCrood); break;
		case 9: textureColor = texture(u_Textures[9], v_TexCrood); break;
		case 10: textureColor = texture(u_Textures[10], v_TexCrood); break;
		case 11: textureColor = texture(u_Textures[11], v_TexCrood); break;
		case 12: textureColor = texture(u_Textures[12], v_TexCrood); break;
		case 13: textureColor = texture(u_Textures[13], v_TexCrood); break;
		case 14: textureColor = texture(u_Textures[14], v_TexCrood); break;
		case 15: textureColor = texture(u_Textures[15], v_TexCrood); break;
		case 16: textureColor = texture(u_Textures[16], v_TexCrood); break;
		case 17: textureColor = texture(u_Textures[17], v_TexCrood); break;
		case 18: textureColor = texture(u_Textures[18], v_TexCrood); break;
		case 19: textureColor = texture(u_Textures[19], v_TexCrood); break;
		default: textureColor = vec4(0.0, 0.0, 0.0, 0.0); break;
	}
	
	color = textureColor * v_Color;	
}