#version 430 core
out vec4 frag_colour;

in vec3 f_pos; // this value becomes interpolated between vertices

uniform samplerCube texture_diffuse;
uniform samplerCube texture_specular;

void main()
{
    frag_colour = texture(texture_diffuse, f_pos);
}