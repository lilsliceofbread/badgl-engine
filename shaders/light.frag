#version 430 core
out vec4 frag_colour;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

void main()
{
    frag_colour = vec4(material.diffuse, 1.0);
}