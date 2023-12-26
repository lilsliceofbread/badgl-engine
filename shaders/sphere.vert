#version 430 core
layout (location = 0) in vec3 v_pos;

out vec3 f_pos;
out vec3 f_normal;
out vec3 f_tex_coord;
out vec3 f_light_pos;

uniform vec3 light_pos;

uniform mat4 mvp;
uniform mat4 model_view;
uniform mat4 view;

void main()
{
    gl_Position = mvp * vec4(v_pos, 1.0f);

    //mat3 normal_matrix = mat3(transpose(inverse(model_view))); // remove translation
    mat3 normal_matrix = mat3(model_view); // remove translation

    f_normal = normal_matrix * normalize(v_pos);
    f_pos = (model_view * vec4(v_pos, 1.0f)).xyz;
    f_light_pos = (view * vec4(light_pos, 1.0f)).xyz;
    f_tex_coord = normalize(v_pos);
}