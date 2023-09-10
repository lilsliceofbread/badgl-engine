#version 330 core
in vec2 tex_coord;

out vec4 frag_colour;

uniform float offset;
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    if(offset > 0)
    {
        frag_colour = texture(texture1, tex_coord);
    }
    else
    {
        frag_colour = texture(texture2, tex_coord);
    }
}