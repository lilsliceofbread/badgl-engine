#ifndef BADGL_MATH_H
#define BADGL_MATH_H

#include <math.h>

#define      GL_PI 3.14159265358979323846f
#define GL_DEG2RAD 0.01745329251994329576f // pi/180

#define math_rad(deg) ((deg) * GL_DEG2RAD)

// align types based on cglm
#if defined(_MSC_VER)
/* do not use alignment for older visual studio versions */
#  if _MSC_VER < 1913 /*  Visual Studio 2017 version 15.6  */
#    define BADGL_ALL_UNALIGNED
#    define BADGL_ALIGN(X) /* no alignment */
#  else
#    define BADGL_ALIGN(X) __declspec(align(X))
#  endif
#else
#  define BADGL_ALIGN(X) __attribute((aligned(X)))
#endif

#ifndef BADGL_ALL_UNALIGNED
#  define BADGL_ALIGN_IF(X) BADGL_ALIGN(X)
#else
#  define BADGL_ALIGN_IF(X) /* no alignment */
#endif

#ifdef __AVX__
#  define BADGL_ALIGN_MAT BADGL_ALIGN(32)
#else
#  define BADGL_ALIGN_MAT BADGL_ALIGN(16)
#endif

typedef union vec2
{
    float data[2];
    struct
    {
        float x;
        float y;
    };

    struct
    {
        float u;
        float v;
    };
} vec2;

typedef union vec3
{
    float data[3];

    struct
    {
        float x;
        float y;
        float z;
    };

    struct
    {
        float r;
        float g;
        float b;
    };
} vec3;

typedef union BADGL_ALIGN_IF(16) vec4
{
    float data[4];

    struct
    {
        float x;
        float y;
        float z;
        float w;
    };

    struct
    {
        float r;
        float g;
        float b;
        float a;
    };
} vec4;

typedef union BADGL_ALIGN_MAT mat4
{
    float data[16]; 
    vec4 cols[4];
    struct
    {
        float m11;
        float m21;
        float m31;
        float m41;

        float m12;
        float m22;
        float m32;
        float m42;

        float m13;
        float m23;
        float m33;
        float m43;

        float m14;
        float m24;
        float m34;
        float m44;
    };
} mat4;

void vec3_copy(vec3 src, vec3* dest);
vec3 vec3_zero();
float vec3_dot(vec3 v1, vec3 v2);
vec3 vec3_scale(vec3 vec, float s);
vec3 vec3_add_scalar(vec3 vec, float s);
vec3 vec3_add(vec3 v1, vec3 v2);
vec3 vec3_sub(vec3 v1, vec3 v2);
vec3 vec3_norm(vec3 vec);
vec3 vec_cross(vec3 v1, vec3 v2); // only exists for vec3

void vec2_copy(vec2 src, vec2* dest);
vec2 vec2_zero();
float vec2_dot(vec2 v1, vec2 v2);
vec2 vec2_scale(vec2 vec, float s);
vec2 vec2_add_scalar(vec2 vec, float s);
vec2 vec2_add(vec2 v1, vec2 v2);
vec2 vec2_sub(vec2 v1, vec2 v2);
vec2 vec2_norm(vec2 vec);

// opengl is column major so in memory matrices are flipped from traditional matrices
mat4 mat4_mul(mat4 m1, mat4 m2);
mat4 mat4_identity();
mat4 mat4_zero();
mat4 mat4_transpose(mat4 mat);
mat4 mat4_scale(mat4 mat, vec3 s);
mat4 mat4_trans(mat4 mat, vec3 t);
mat4 mat4_rotate_x(mat4 mat, float a); // not figuring out arbitrary axis rotation
mat4 mat4_rotate_y(mat4 mat, float a);
mat4 mat4_rotate_z(mat4 mat, float a);
mat4 mat_perspective_fov(float fov, float aspect, float znear, float zfar);
mat4 mat_look_at(vec3 t, vec3 k, vec3 i); // k is dir or "z axis", i is right or "x axis" and t is pos

#endif