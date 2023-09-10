#version 330 core
out vec4 frag_colour;

in vec3 vert_colour;

void main()
{
    frag_colour = vec4(vert_colour, 1.0f);
}