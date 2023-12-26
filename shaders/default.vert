#version 430 core
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec2 v_uv;

out vec3 f_pos;
out vec3 f_normal;
out vec3 f_light_pos;

uniform vec3 light_pos;

uniform mat4 mvp;
uniform mat4 model_view;
uniform mat4 view;

void main()
{
    gl_Position = mvp * vec4(v_pos, 1.0);

    mat3 normal_matrix = mat3(model_view); // remove translation

    f_normal = normalize(normal_matrix * v_normal);
    f_pos = (model_view * vec4(v_pos, 1.0f)).xyz;
    f_light_pos = (view * vec4(light_pos, 1.0f)).xyz;
}