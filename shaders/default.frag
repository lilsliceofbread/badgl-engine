#version 460 core

in vec2 f_uv;
in vec3 f_normal;

out vec4 frag_colour;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

void main()
{
    frag_colour = texture(texture_diffuse1, f_uv);
    //frag_colour = vec4(f_normal, 1.0f);
}