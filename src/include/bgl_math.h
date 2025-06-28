#ifndef BGL_MATH_H
#define BGL_MATH_H

// TODO: add funcs for mat3 and mat2, optimise others

#include <math.h>
#include <types.h>

#define      BGL_PI 3.14159265358979323846f
#define BGL_DEG2RAD 0.01745329251994329576f // PI / 180

#define RADIANS(deg) ((deg) * BGL_DEG2RAD)

#define CLAMP(val, lower, upper) ((val) < (lower)) ? (lower) : ((val) > (upper)) ? (upper) : (val)

#define VEC4TOVEC3(vector) (vec3){vector.x, vector.y, vector.z}
#define VEC3TOVEC4(vector, w) (vec4){vector.x, vector.y, vector.z, w}

#define VEC4(x, y, z, w) (vec4){(x), (y), (z), (w)}
#define VEC3(x, y, z) (vec3){(x), (y), (z)}
#define VEC2(x, y) (vec2){(x), (y)}

typedef union vec2
{
    f32 data[2];
    struct
    {
        f32 x, y;
    };
    struct
    {
        f32 u, v;
    };
} vec2;

typedef union vec3
{
    f32 data[3];
    struct
    {
        f32 x, y, z;
    };
    struct
    {
        f32 r, g, b;
    };
} vec3;

typedef union vec4
{
    f32 data[4];
    struct
    {
        f32 x, y, z, w;
    };
    struct
    {
        f32 r, g, b, a;
    };
} vec4;

/* matrices are column major memory order */
typedef union mat2
{
    f32 data[4]; 
    vec2 cols[2];
    struct
    {
        f32 m11, m21;
        f32 m12, m22;
    };
} mat2;

typedef union mat3
{
    f32 data[9]; 
    vec3 cols[3];
    struct
    {
        f32 m11, m21, m31;
        f32 m12, m22, m32;
        f32 m13, m23, m33;
    };
} mat3;

typedef union mat4
{
    f32 data[16]; 
    vec4 cols[4];
    struct
    {
        f32 m11, m21, m31, m41;
        f32 m12, m22, m32, m42;
        f32 m13, m23, m33, m43;
        f32 m14, m24, m34, m44;
    };
} mat4;

f32 vec3_dot(vec3 v1, vec3 v2);
vec3 vec3_scale(vec3 vec, f32 s);
vec3 vec3_add_scalar(vec3 vec, f32 s);
vec3 vec3_add(vec3 v1, vec3 v2);
vec3 vec3_sub(vec3 v1, vec3 v2);
void vec3_norm(vec3* out);
vec3 vec_cross(vec3 v1, vec3 v2);

f32 vec2_dot(vec2 v1, vec2 v2);
vec2 vec2_scale(vec2 vec, f32 s);
vec2 vec2_add_scalar(vec2 vec, f32 s);
vec2 vec2_add(vec2 v1, vec2 v2);
vec2 vec2_sub(vec2 v1, vec2 v2);
void vec2_norm(vec2* out);

// opengl is column major so when initialising look flipped from traditional matrices
mat4 mat4_zero(void);
void mat4_identity(mat4* out);
void mat4_mul(mat4* out, mat4 m1, mat4 m2);
void mat4_transpose(mat4* out, mat4 mat);
void mat4_scale(mat4* out, vec3 s);
void mat4_scale_scalar(mat4* out, f32 s);
void mat4_trans(mat4* out, vec3 t);
void mat4_rotate_x(mat4* out, f32 a); // not figuring out arbitrary axis rotation
void mat4_rotate_y(mat4* out, f32 a);
void mat4_rotate_z(mat4* out, f32 a);

void mat_perspective_fov(mat4* out, f32 fov, f32 aspect, f32 near, f32 far);
void mat_perspective_frustrum(mat4* out, f32 near, f32 far, f32 left, f32 right, f32 bottom, f32 top);
void mat_orthographic_frustrum(mat4* out, f32 near, f32 far, f32 left, f32 right, f32 bottom, f32 top);
void mat_look_at(mat4* out, vec3 t, vec3 k, vec3 i); // t is pos or translation vec, k is dir or "z axis", i is right or "x axis"

#endif
