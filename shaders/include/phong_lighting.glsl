#ifndef BGL_PHONG_LIGHTING_GLSL
#define BGL_PHONG_LIGHTING_GLSL

#define BAND_RANGE 0.002 // 0.5 / 255, range of noise to remove banding

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 compute_dir_light(DirLight light, vec3 normal, vec3 frag_pos)
{
    vec3 view_light_dir = (mat3(view) * normalize(-light.dir));

    /* ambient */

    vec3 ambient = (material.ambient * texture(texture_diffuse, fs_in.tex_coord).xyz) * light.ambient.xyz;

    /* diffuse */

    float diffuse_strength = max(dot(normal, view_light_dir), 0.0);
    vec3 diffuse = (diffuse_strength * material.diffuse * texture(texture_diffuse, fs_in.tex_coord).xyz) * light.diffuse.xyz;

    /* specular */

    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-view_light_dir, normal);
    float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (specular_strength * material.specular * texture(texture_specular, fs_in.tex_coord).xyz) * light.specular.xyz;

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

    vec3 ambient = (material.ambient * texture(texture_diffuse, fs_in.tex_coord).xyz) * light.ambient.xyz;

    /* diffuse */

    vec3 light_dir = normalize(view_light_pos - frag_pos);
    float diffuse_strength = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = (diffuse_strength * material.diffuse * texture(texture_diffuse, fs_in.tex_coord).xyz) * light.diffuse.xyz;

    /* specular */

    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = (specular_strength * material.specular * texture(texture_specular, fs_in.tex_coord).xyz) * light.specular.xyz;

    return attenuation * (ambient + diffuse + specular);
}

// TODO: fix this scuffed function to take in arguments instead
vec3 compute_phong_light()
{
    vec3 normal = normalize(fs_in.normal);

    vec3 result = compute_dir_light(dir_light, normal, fs_in.pos);
    for(int i = 0; i < light_count; i++)
    {
        result += compute_point_light(light_buffer[i], normal, fs_in.pos, fs_in.world_pos);
    }

    result += mix(-BAND_RANGE, BAND_RANGE, rand(gl_FragCoord.xy)); // noise to fix banding
    
    return result;
}

#endif
