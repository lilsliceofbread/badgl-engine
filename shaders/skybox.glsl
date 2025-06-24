#type vertex
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_normal;

out vec3 f_pos;

uniform mat4 mvp; // mvp without the translation (or the model)

void main()
{
    f_pos = v_pos;

    vec4 corrected_pos = mvp * vec4(v_pos, 1.0);
    gl_Position = corrected_pos.xyww; // ensure all NDC z values are 1.0
}

#type fragment
#include "include/defines.glsl"

out vec4 frag_colour;

in vec3 f_pos;

uniform samplerCube BGL_GLSL_TEXTURE_DIFFUSE;
//uniform samplerCube BGL_GLSL_TEXTURE_SPECULAR;

void main()
{
    frag_colour = texture(texture_diffuse, f_pos);
}
