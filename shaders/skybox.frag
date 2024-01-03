#version 430 core
out vec4 frag_colour;

in vec3 f_pos; // this value becomes interpolated between vertices

uniform samplerCube cubemap;

void main()
{
    frag_colour = texture(cubemap, f_pos);
}