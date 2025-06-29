#include "include/defines.glsl"
#include "include/phong_types.glsl"

out vec4 frag_colour;

in VSOut vs_out;

layout(std140, binding = 0) uniform Lights
{
    Light light_buffer[BGL_GLSL_MAX_POINT_LIGHTS];
    int light_count;
};
uniform DirLight dir_light;
uniform Material material;
uniform mat4 view;
uniform sampler2D BGL_GLSL_TEXTURE_DIFFUSE;
uniform sampler2D BGL_GLSL_TEXTURE_SPECULAR;

#include "include/phong_lighting.glsl"

void main()
{
    vec3 phong = compute_phong_light(vs_out.normal, vs_out.frag_pos, vs_out.world_pos, vs_out.tex_coord);
    frag_colour = vec4(phong, 1.0);
}
