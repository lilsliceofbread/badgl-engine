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
    vec2 uv;
    vec3 light_pos;
} fs_in;

uniform Light light;
uniform Material material;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

#define BAND_RANGE 0.002 // 0.5 / 255, range of noise to remove banding

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 compute_point_light(Light light, vec3 normal, vec3 frag_pos, vec3 light_pos)
{
    /* attenuation */

    float dist = length(light.pos - fs_in.world_pos);
    float attenuation = 1.0 / (light.attenuation.x * (dist * dist) +
                               light.attenuation.y * dist          +
                               light.attenuation.z);

    /* ambient */

    vec3 ambient = (material.ambient * texture(texture_diffuse, fs_in.uv).xyz) * light.ambient;

    /* diffuse */

    vec3 light_dir = normalize(light_pos - frag_pos);
    float diffuse_strength = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = (diffuse_strength * material.diffuse * texture(texture_diffuse, fs_in.uv).xyz) * light.diffuse;

    /* specular */

    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (specular_strength * material.specular * texture(texture_specular, fs_in.uv).xyz) * light.specular;

    return attenuation * (ambient + diffuse + specular);
}

void main()
{
    vec3 normal = normalize(fs_in.normal);
    vec3 frag_pos = fs_in.pos;
    vec3 light_pos = fs_in.light_pos;

    vec3 result = vec3(0.0);
    result += compute_point_light(light, normal, frag_pos, light_pos);

    result += mix(-BAND_RANGE, BAND_RANGE, rand(gl_FragCoord.xy)); // noise to fix banding
    frag_colour = vec4(result, 1.0);
}