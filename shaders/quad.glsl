#type vertex
layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec2 v_uv;

out vec2 f_uv;

void main()
{
    gl_Position = vec4(v_pos, 1.0);
    f_uv = v_uv;
}

#type fragment
out vec4 frag_colour;

in vec2 f_uv;

uniform sampler2D quad_texture;

void main()
{
    frag_colour = texture(quad_texture, f_uv);
}
