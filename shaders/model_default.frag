#version 430 core

in vec2 f_uv;
in vec3 f_normal;

out vec4 frag_colour;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

void main()
{
    frag_colour = texture(texture_diffuse, f_uv);
    //frag_colour = vec4(f_normal, 1.0f);
}