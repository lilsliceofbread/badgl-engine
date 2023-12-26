#version 430 core

in vec3 f_pos;
in vec3 f_normal;
in vec2 f_uv;
in vec3 f_light_pos;

out vec4 frag_colour;

uniform vec3 light_colour;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

void main()
{
    vec3 frag_pos = f_pos;
    vec3 normal = normalize(f_normal);

    float ambient_multiplier = 0.1;
    float specular_multiplier = 0.5;

    /* ambient */

    vec3 ambient = ambient_multiplier * light_colour;

    /* diffuse */

    vec3 light_dir = normalize(f_light_pos - frag_pos);
    float diffuse_strength = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = diffuse_strength * light_colour;

    /* specular */

    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_multiplier * specular_strength * light_colour;

    vec3 result = (ambient + diffuse + specular) * texture(texture_diffuse, f_uv).xyz;
    frag_colour = vec4(result, 1.0);
}