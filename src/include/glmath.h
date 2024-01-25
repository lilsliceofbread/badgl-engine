#ifndef BGL_MATH_H
#define BGL_MATH_H

// TODO: add funcs for mat3 and mat2, optimise others

#include <math.h>

#define      GL_PI 3.14159265358979323846f
#define GL_DEG2RAD 0.01745329251994329576f // PI / 180

#define RADIANS(deg) ((deg) * GL_DEG2RAD)

#define VEC4TOVEC3(vector) (vec3){vector.x, vector.y, vector.z}
#define VEC3TOVEC4(vector, w) (vec4){vector.x, vector.y, vector.z, w}

#define VEC4(x, y, z, w) (vec4){(x), (y), (z), (w)}
#define VEC3(x, y, z) (vec3){(x), (y), (z)}
#define VEC2(x, y) (vec2){(x), (y)}

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

typedef union vec4
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

typedef union mat4
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

typedef union mat3
{
    float data[9]; 
    vec3 cols[3];
    struct
    {
        float m11;
        float m21;
        float m31;

        float m12;
        float m22;
        float m32;

        float m13;
        float m23;
        float m33;
    };
} mat3;

typedef union mat2
{
    float data[4]; 
    vec2 cols[2];
    struct
    {
        float m11;
        float m21;

        float m12;
        float m22;
    };
} mat2;

float vec3_dot(vec3 v1, vec3 v2);
vec3 vec3_scale(vec3 vec, float s);
vec3 vec3_add_scalar(vec3 vec, float s);
vec3 vec3_add(vec3 v1, vec3 v2);
vec3 vec3_sub(vec3 v1, vec3 v2);
void vec3_norm(vec3* out);
vec3 vec_cross(vec3 v1, vec3 v2); // only exists for vec3

float vec2_dot(vec2 v1, vec2 v2);
vec2 vec2_scale(vec2 vec, float s);
vec2 vec2_add_scalar(vec2 vec, float s);
vec2 vec2_add(vec2 v1, vec2 v2);
vec2 vec2_sub(vec2 v1, vec2 v2);
void vec2_norm(vec2* out);

// opengl is column major so in memory matrices are flipped from traditional matrices
mat4 mat4_zero(void);
void mat4_identity(mat4* out);
void mat4_mul(mat4* out, mat4 m1, mat4 m2);
void mat4_transpose(mat4* out, mat4 mat);
void mat4_scale(mat4* out, vec3 s);
void mat4_scale_scalar(mat4* out, float s);
void mat4_trans(mat4* out, vec3 t);
void mat4_rotate_x(mat4* out, float a); // not figuring out arbitrary axis rotation
void mat4_rotate_y(mat4* out, float a);
void mat4_rotate_z(mat4* out, float a);

void mat_perspective_fov(mat4* out, float fov, float aspect, float near, float far);
void mat_perspective_frustrum(mat4* out, float near, float far, float left, float right, float bottom, float top);
void mat_orthographic_frustrum(mat4* out, float near, float far, float left, float right, float bottom, float top);
void mat_look_at(mat4* out, vec3 t, vec3 k, vec3 i); // t is pos or translation vec, k is dir or "z axis", i is right or "x axis"

#endif