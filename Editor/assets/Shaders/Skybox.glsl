#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;


uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_Position;

void main() {

    v_Position = normalize(a_Position);
    mat4 view = mat4(mat3(u_View));
    gl_Position =  u_Projection * view * vec4(a_Position, 1.0); 
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;
layout(location = 1) out int entityID;

uniform samplerCube u_SkyBox;

in vec3 v_Position;

void main() {

    vec3 position = vec3(v_Position.x, -v_Position.y, v_Position.z);
    color = texture(u_SkyBox, position);
    //color = vec4(v_Position, 1.0);
    entityID = -1;
}
