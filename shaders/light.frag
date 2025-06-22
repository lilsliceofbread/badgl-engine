#version 420 core
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
    /* multiply diffuse by arbitrary value because it looks nicer */
    frag_colour = vec4(2.0 * material.diffuse, 1.0);
}
