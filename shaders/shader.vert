#version 330 core
layout (location = 0) in vec3 v_pos;

out vec3 vert_colour;

uniform float vert_offset;

void main()
{
    vec3 vertex_pos = vec3(v_pos.x + vert_offset, v_pos.yz - vert_offset);
    gl_Position = vec4(vertex_pos, 1.0f);
    vert_colour = vertex_pos;
}