#version 430 core
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_uv;

struct Light {
    vec3 pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation; // attenuation constants - quadratic, linear, constant
};

out VsOut {
    vec3 world_pos;
    vec3 pos;
    vec3 normal;
    vec2 uv;
    vec3 light_pos;
} vs_out;

uniform Light light;
uniform mat4 mvp;
uniform mat4 model_view;
uniform mat4 model;
uniform mat4 view;

void main()
{
    gl_Position = mvp * vec4(v_pos, 1.0);

    mat3 normal_matrix = mat3(transpose(inverse(model_view))); // remove translation
    //mat3 normal_matrix = mat3(model_view); // remove translation

    vs_out.world_pos = (model * vec4(v_pos, 1.0f)).xyz;
    vs_out.pos = (model_view * vec4(v_pos, 1.0f)).xyz;
    vs_out.normal = normalize(normal_matrix * v_normal);
    vs_out.uv = v_uv;
    vs_out.light_pos = (view * vec4(light.pos, 1.0f)).xyz;
} // all vertices are divided by w after vertex shader