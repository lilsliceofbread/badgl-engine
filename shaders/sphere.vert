#include "include/defines.glsl"

/* need to define this for phong_lighting.glsl so it uses vec3 for tex coords instead of vec2 */
#define tex_coord_t vec3
#include "include/phong_types.glsl"

layout (location = 0) in vec3 v_pos;

out VSOut vs_out;

layout(std140, binding = 0) uniform Lights
{
    Light light_buffer[BGL_GLSL_MAX_POINT_LIGHTS];
    int light_count;
};
uniform mat4 mvp;
uniform mat4 model_view;
uniform mat4 model;
uniform mat4 view;

void main()
{
    gl_Position = mvp * vec4(v_pos, 1.0f);

    mat3 normal_matrix = mat3(transpose(inverse(model_view))); // remove translation

    vs_out.frag_pos = (model_view * vec4(v_pos, 1.0f)).xyz;
    vs_out.world_pos = (model * vec4(v_pos, 1.0f)).xyz;
    vs_out.normal = normal_matrix * normalize(v_pos); // transform vertex normals to match model
    vs_out.tex_coord = normalize(v_pos);
}
