#version 430 core
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_uv;

out vec3 f_normal;

uniform mat4 model;
uniform mat4 vp;

void main()
{
    gl_Position = vp * model * vec4(v_pos, 1.0f);

    f_normal = v_normal;
} // all vertices are divided by w after vertex shader