#version 460 core

in vec3 f_normal;

out vec4 frag_colour;

uniform samplerCube cubemap;

void main()
{
    frag_colour = texture(cubemap, f_normal);
}