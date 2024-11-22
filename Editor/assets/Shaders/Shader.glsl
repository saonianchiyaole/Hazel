#type vertex

#version 330 core

layout(location = 0) in vec3 a_Position;		

uniform mat4 u_Transform;
uniform mat4 u_View;
uniform mat4 u_Projection;
	

void main()
{		

	gl_Position = u_Projection * u_View * u_Transform *  vec4(a_Position, 1.0);
}


#type fragment

#version 330 core

layout(location = 0) out vec4 color;	

void main()
{
	color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//color = vec4(v_Position, 1.0f);
}