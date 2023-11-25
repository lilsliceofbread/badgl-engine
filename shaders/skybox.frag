#version 430 core

in vec3 f_pos; // this value becomes interpolated between vertices

out vec4 frag_colour;

uniform samplerCube cubemap;

void main()
{
    frag_colour = texture(cubemap, f_pos);
}