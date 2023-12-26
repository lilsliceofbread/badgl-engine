#version 430 core
layout (location = 0) in vec3 v_pos;

out vec3 f_pos;

uniform mat4 mvp; // mvp without the translation (or the model)

void main()
{
    f_pos = v_pos;

    vec4 corrected_pos = mvp * vec4(v_pos, 1.0);
    gl_Position = corrected_pos.xyww; // ensure all NDC z values are 1.0
}