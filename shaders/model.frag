#version 420 core
out vec4 frag_colour;

#define MAX_LIGHTS 32

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec4 pos;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation; // attenuation constants - quadratic, linear, constant
};

struct DirLight {
    vec3 dir;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in VsOut {
    vec3 world_pos;
    vec3 pos;
    vec3 normal;
    vec2 uv;
} fs_in;

layout(std140, binding = 0) uniform Lights
{
    Light light_buffer[MAX_LIGHTS];
    int light_count;
};
uniform DirLight dir_light;
uniform Material material;
uniform mat4 view;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

#define BAND_RANGE 0.002 // 0.5 / 255, range of noise to remove banding

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 compute_dir_light(DirLight light, vec3 normal, vec3 frag_pos)
{
    vec3 view_light_dir = (mat3(view) * normalize(-light.dir));

    /* ambient */

    vec3 ambient = (material.ambient * texture(texture_diffuse, fs_in.uv).xyz) * light.ambient.xyz;

    /* diffuse */

    float diffuse_strength = max(dot(normal, view_light_dir), 0.0);
    vec3 diffuse = (diffuse_strength * material.diffuse * texture(texture_diffuse, fs_in.uv).xyz) * light.diffuse.xyz;

    /* specular */

    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-view_light_dir, normal);
    float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (specular_strength * material.specular * texture(texture_specular, fs_in.uv).xyz) * light.specular.xyz;

    return (ambient + diffuse + specular);
}

vec3 compute_point_light(Light light, vec3 normal, vec3 frag_pos, vec3 world_pos)
{
    vec3 view_light_pos = (view * light.pos).xyz;

    /* attenuation */

    float dist = length(light.pos.xyz - world_pos);
    float attenuation = 1.0 / (light.attenuation.x * (dist * dist) +
                               light.attenuation.y * dist          +
                               light.attenuation.z);

    /* ambient */

    vec3 ambient = (material.ambient * texture(texture_diffuse, fs_in.uv).xyz) * light.ambient.xyz;

    /* diffuse */

    vec3 light_dir = normalize(view_light_pos - frag_pos);
    float diffuse_strength = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = (diffuse_strength * material.diffuse * texture(texture_diffuse, fs_in.uv).xyz) * light.diffuse.xyz;

    /* specular */

    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (specular_strength * material.specular * texture(texture_specular, fs_in.uv).xyz) * light.specular.xyz;

    return attenuation * (ambient + diffuse + specular);
}

void main()
{
    vec3 normal = normalize(fs_in.normal);

    vec3 result = compute_dir_light(dir_light, normal, fs_in.pos);
    for(int i = 0; i < light_count; i++)
    {
        result += compute_point_light(light_buffer[i], normal, fs_in.pos, fs_in.world_pos);
    }

    result += mix(-BAND_RANGE, BAND_RANGE, rand(gl_FragCoord.xy)); // noise to fix banding
    frag_colour = vec4(result, 1.0);
}
