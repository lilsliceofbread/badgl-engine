#version 460 core
in vec2 uv;
in vec3 normal;

out vec4 frag_colour;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse3;
uniform sampler2D texture_specular1;
uniform sampler2D texture_specular2;

void main()
{
    float test = dot(vec4(normal, 1.0f), texture(texture_specular1, uv));
    frag_colour = test * texture(texture_diffuse1, uv);
    //frag_colour = vec4(normal, 1.0f);
}