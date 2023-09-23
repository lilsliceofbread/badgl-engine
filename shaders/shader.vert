#version 330 core
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec2 v_tex_coord;

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 vp;

void main()
{
    gl_Position = vp * model * vec4(v_pos, 1.0f);
    tex_coord = v_tex_coord;
}