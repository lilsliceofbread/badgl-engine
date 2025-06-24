#type vertex
layout (location = 0) in vec3 v_pos;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(v_pos, 1.0);
}

#type fragment
#include "include/phong_types.glsl"

out vec4 frag_colour;

uniform Material material;

void main()
{
    /* multiply diffuse by arbitrary value because it looks nicer */
    frag_colour = vec4(2.0 * material.diffuse, 1.0);
}
