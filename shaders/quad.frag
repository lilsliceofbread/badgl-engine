#version 430 core
out vec4 frag_colour;

in vec2 f_uv;

uniform sampler2D quad_texture;

void main()
{
    frag_colour = texture(quad_texture, f_uv);
}