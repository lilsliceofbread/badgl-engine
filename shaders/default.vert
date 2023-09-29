#version 460 core
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_uv;

out vec2 uv;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    gl_Position = proj * view * model * vec4(v_pos, 1.0f);
    //gl_Position = vec4(v_pos, 1.0f);
    uv = v_uv;
    normal = v_normal;
}