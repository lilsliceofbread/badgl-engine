#version 330 core
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec2 v_tex_coord;

out vec2 tex_coord;

uniform float offset;

void main()
{
    vec3 offset_vert_pos = vec3(v_pos.x + offset, v_pos.yz - offset);
    gl_Position = vec4(offset_vert_pos, 1.0f);
    tex_coord = v_tex_coord;
}