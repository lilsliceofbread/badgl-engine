#version 430 core
out vec4 frag_colour;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation; // attenuation constants - quadratic, linear, constant
};

in VsOut {
    vec3 world_pos;
    vec3 pos;
    vec3 normal;
    vec3 light_pos;
} fs_in;

uniform Light light;
uniform Material material;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

#define BAND_RANGE 0.002 // 0.5 / 255, range of noise to remove banding

void main()
{
    vec3 normal = normalize(fs_in.normal);
    vec3 frag_pos = fs_in.pos;
    vec3 light_pos = fs_in.light_pos;

    /* attenuation */

    float dist = length(light.pos - fs_in.world_pos);
    vec3 att_vec = light.attenuation;
    float attenuation = 1.0 / (att_vec.x * (dist * dist) + att_vec.y * dist + att_vec.z);

    /* ambient */

    vec3 ambient = material.ambient * light.ambient;

    /* diffuse */

    vec3 light_dir = normalize(light_pos - frag_pos);
    float diffuse_strength = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = (diffuse_strength * material.diffuse) * light.diffuse;

    /* specular */

    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (specular_strength * material.specular) * light.specular;

    vec3 result = (ambient + diffuse + specular) * attenuation;
    result += mix(-BAND_RANGE, BAND_RANGE, rand(gl_FragCoord.xy)); // noise to fix banding
    frag_colour = vec4(result, 1.0);
}