#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;		
layout(location = 1) in vec2 a_TexCoord;


out vec2 v_TexCoord;

void main()
{	
    v_TexCoord = a_TexCoord;	
    gl_Position = vec4(a_Position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;


uniform sampler2D u_Texture;
in vec2 v_TexCoord;

void main(){

    float gamma     = 2.2;
	float pureWhite = 1.0;

    vec4 color = texture(u_Texture, v_TexCoord);

    o_Color = color;
    //o_Color = vec4(pow(color.rgb, vec3(1.0 / gamma)), 1.0);

}

