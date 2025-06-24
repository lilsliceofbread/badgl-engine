#include "include/defines.glsl"
#include "include/phong_types.glsl"

out vec4 frag_colour;

in VsOut {
    vec3 world_pos;
    vec3 pos;
    vec3 normal;
    vec3 tex_coord;
} fs_in;

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
    frag_colour = vec4(compute_phong_light(), 1.0);
}
