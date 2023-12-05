#version 430 core
layout (location = 0) in vec3 v_pos;

out vec3 f_pos;

uniform mat4 vp; // vp without the translation

void main()
{
    f_pos = v_pos;

    vec4 corrected_pos = vp * vec4(v_pos, 1.0f);
    gl_Position = corrected_pos.xyww; // ensure all NDC z values are 1.0
} // all vertices are divided by w after vertex shader