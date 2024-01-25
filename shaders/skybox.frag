#version 430 core
out vec4 frag_colour;

in vec3 f_pos;

uniform samplerCube texture_diffuse;
uniform samplerCube texture_specular; // TODO: make it so that this is not necessary

void main()
{
    frag_colour = texture(texture_diffuse, f_pos);
}