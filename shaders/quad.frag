#version 430 core

in vec2 f_uv;

out vec4 frag_colour;

uniform sampler2D quad_texture;

void main()
{
    frag_colour = texture(quad_texture, f_uv);
}