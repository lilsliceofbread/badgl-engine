#version 330 core
in vec2 tex_coord;

out vec4 frag_colour;

uniform sampler2D texture1;

void main()
{
    frag_colour = texture(texture1, tex_coord);
}