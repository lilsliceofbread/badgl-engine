#version 330 core
in vec2 uv;

out vec4 frag_colour;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

void main()
{
    frag_colour = texture(texture_diffuse1, uv);
}