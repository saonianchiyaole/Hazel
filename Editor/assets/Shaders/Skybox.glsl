#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform Camera
{
	uniform mat4 u_View;
    uniform mat4 u_Projection;
	vec3 cameraPosition;
};

layout(location = 0) out vec3 v_Position;

void main() {

    v_Position = a_Position;
    mat4 view = mat4(mat3(u_View));
    gl_Position =  u_Projection * view * vec4(a_Position, 1.0); 
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

layout(binding = 1) uniform samplerCube u_SkyBox;

layout(location = 0) in vec3 v_Position;

void main() {

    vec3 position = vec3(v_Position.x, -v_Position.y, v_Position.z);
    color = textureLod(u_SkyBox, position, 0.1);
    //color = vec4(v_Position, 1.0);    
}
