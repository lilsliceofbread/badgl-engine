#include "include/defines.glsl"

/* need to define this for phong_lighting.glsl so it uses vec3 for tex coords instead of vec2 */
#define tex_coord_t vec3
#include "include/phong_types.glsl"

out vec4 frag_colour;

in VSOut vs_out;

layout(std140, binding = 0) uniform Lights
{
    Light light_buffer[BGL_GLSL_MAX_LIGHTS];
    int light_count;
};
uniform DirLight dir_light;
uniform Material material;
uniform mat4 view;
uniform samplerCube BGL_GLSL_TEXTURE_DIFFUSE;
uniform samplerCube BGL_GLSL_TEXTURE_SPECULAR;

#include "include/phong_lighting.glsl"

void main()
{
    vec3 phong = compute_phong_light(vs_out.normal, vs_out.frag_pos, vs_out.world_pos, vs_out.tex_coord);
    frag_colour = vec4(phong, 1.0);
}
