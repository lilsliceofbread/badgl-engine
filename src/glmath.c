#include "glmath.h"

#include <stdlib.h>
#include <string.h>

float vec3_dot(vec3 v1, vec3 v2)
{
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

vec3 vec3_scale(vec3 vec, float s)
{
    vec3 new_vec;
    new_vec.x = s * vec.x;
    new_vec.y = s * vec.y;
    new_vec.z = s * vec.z;

    return new_vec;
}

vec3 vec3_add_scalar(vec3 vec, float s)
{
    vec3 new_vec;
    new_vec.x = vec.x + s;
    new_vec.y = vec.y + s;
    new_vec.z = vec.z + s;

    return new_vec;
}

vec3 vec3_add(vec3 v1, vec3 v2)
{
    vec3 new_vec;
    new_vec.x = v1.x + v2.x;
    new_vec.y = v1.y + v2.y;
    new_vec.z = v1.z + v2.z;

    return new_vec;
}

vec3 vec3_sub(vec3 v1, vec3 v2)
{
    vec3 new_vec;
    new_vec.x = v1.x - v2.x;
    new_vec.y = v1.y - v2.y;
    new_vec.z = v1.z - v2.z;

    return new_vec;
}

void vec3_norm(vec3* out)
{
    vec3 tmp = *out;
    float denom = sqrtf(vec3_dot(tmp, tmp));

    if(denom == 0.0f) // prevent divide by zero
    {
        out->x = out->y = out->z = 0.0f;
        return;
    }

    *out = vec3_scale(tmp, 1.0f / denom);
}

vec3 vec_cross(vec3 v1, vec3 v2)
{
    vec3 new_vec;
    new_vec.x = v1.y * v2.z - v1.z * v2.y;
    new_vec.y = v1.z * v2.x - v1.x * v2.z;
    new_vec.z = v1.x * v2.y - v1.y * v2.x;

    return new_vec;
}

float vec2_dot(vec2 v1, vec2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

vec2 vec2_scale(vec2 vec, float s)
{
    vec2 new_vec;
    new_vec.x = s * vec.x;
    new_vec.y = s * vec.y;

    return new_vec;
}

vec2 vec2_add_scalar(vec2 vec, float s)
{
    vec2 new_vec;
    new_vec.x = vec.x + s;
    new_vec.y = vec.y + s;

    return new_vec;
}

vec2 vec2_add(vec2 v1, vec2 v2)
{
    vec2 new_vec;
    new_vec.x = v1.x + v2.x;
    new_vec.y = v1.y + v2.y;

    return new_vec;
}

vec2 vec2_sub(vec2 v1, vec2 v2)
{
    vec2 new_vec;
    new_vec.x = v1.x - v2.x;
    new_vec.y = v1.y - v2.y;

    return new_vec;
}

void vec2_norm(vec2* out)
{
    vec2 tmp = *out;
    float denom = sqrtf(vec2_dot(tmp, tmp));

    if(denom == 0.0f) // prevent divide by zero
    {
        out->x = out->y = 0.0f;
        return;
    }

    *out = vec2_scale(tmp, 1.0f / denom);
}

// inefficient lol, has to copy 2 whole matrices + no SIMD stuff
void mat4_mul(mat4* out, mat4 m1, mat4 m2)
{
    out->m11 = m1.m11 * m2.m11 + m1.m12 * m2.m21 + m1.m13 * m2.m31 + m1.m14 * m2.m41;
    out->m21 = m1.m21 * m2.m11 + m1.m22 * m2.m21 + m1.m23 * m2.m31 + m1.m24 * m2.m41;
    out->m31 = m1.m31 * m2.m11 + m1.m32 * m2.m21 + m1.m33 * m2.m31 + m1.m34 * m2.m41;
    out->m41 = m1.m41 * m2.m11 + m1.m42 * m2.m21 + m1.m43 * m2.m31 + m1.m44 * m2.m41;

    out->m12 = m1.m11 * m2.m12 + m1.m12 * m2.m22 + m1.m13 * m2.m32 + m1.m14 * m2.m42;
    out->m22 = m1.m21 * m2.m12 + m1.m22 * m2.m22 + m1.m23 * m2.m32 + m1.m24 * m2.m42;
    out->m32 = m1.m31 * m2.m12 + m1.m32 * m2.m22 + m1.m33 * m2.m32 + m1.m34 * m2.m42;
    out->m42 = m1.m41 * m2.m12 + m1.m42 * m2.m22 + m1.m43 * m2.m32 + m1.m44 * m2.m42;

    out->m13 = m1.m11 * m2.m13 + m1.m12 * m2.m23 + m1.m13 * m2.m33 + m1.m14 * m2.m43;
    out->m23 = m1.m21 * m2.m13 + m1.m22 * m2.m23 + m1.m23 * m2.m33 + m1.m24 * m2.m43;
    out->m33 = m1.m31 * m2.m13 + m1.m32 * m2.m23 + m1.m33 * m2.m33 + m1.m34 * m2.m43;
    out->m43 = m1.m41 * m2.m13 + m1.m42 * m2.m23 + m1.m43 * m2.m33 + m1.m44 * m2.m43;

    out->m14 = m1.m11 * m2.m14 + m1.m12 * m2.m24 + m1.m13 * m2.m34 + m1.m14 * m2.m44;
    out->m24 = m1.m21 * m2.m14 + m1.m22 * m2.m24 + m1.m23 * m2.m34 + m1.m24 * m2.m44;
    out->m34 = m1.m31 * m2.m14 + m1.m32 * m2.m24 + m1.m33 * m2.m34 + m1.m34 * m2.m44;
    out->m44 = m1.m41 * m2.m14 + m1.m42 * m2.m24 + m1.m43 * m2.m34 + m1.m44 * m2.m44;
}

mat4 mat4_zero(void)
{
    mat4 mat = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    return mat;
}

void mat4_identity(mat4* out)
{
    mat4 mat =  {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 
    };
    memcpy(out, mat.data, sizeof(mat));
}

void mat4_transpose(mat4* out, mat4 mat)
{
    out->m11 = mat.m11;
    out->m21 = mat.m12;
    out->m31 = mat.m13;
    out->m41 = mat.m14;

    out->m12 = mat.m21;
    out->m22 = mat.m22;
    out->m32 = mat.m23;
    out->m42 = mat.m24;

    out->m13 = mat.m31;
    out->m23 = mat.m32;
    out->m33 = mat.m33;
    out->m43 = mat.m34;

    out->m14 = mat.m41;
    out->m24 = mat.m42;
    out->m34 = mat.m43;
    out->m44 = mat.m44;
}

void mat4_scale(mat4* out, vec3 s)
{
    mat4 scale_mat = {
        s.x, 0,   0,   0,
        0,   s.y, 0,   0,
        0,   0,   s.z, 0,
        0,   0,   0,   1
    };

    mat4_mul(out, *out, scale_mat);
}

// doesn't scale 4th column
void mat4_scale_scalar(mat4* out, float s)
{
    // optimised method (we know the result of matrix mul will be this)
    out->m11 *= s;
    out->m21 *= s;
    out->m31 *= s;
    out->m41 *= s;

    out->m12 *= s;
    out->m22 *= s;
    out->m32 *= s;
    out->m42 *= s;

    out->m13 *= s;
    out->m23 *= s;
    out->m33 *= s;
    out->m43 *= s;

    // don't do 4th column (w column)
}

void mat4_trans(mat4* out, vec3 t)
{
    mat4 trans_mat = { // * appears transposed because opengl column major memory ordering
        1,   0,   0,   0,
        0,   1,   0,   0,
        0,   0,   1,   0,
        t.x, t.y, t.z, 1
    };

    mat4_mul(out, *out, trans_mat);
}

void mat4_rotate_x(mat4* out, float a) // not figuring out arbitrary axis rotation
{
    float c = cosf(a);
    float s = sinf(a);
    mat4 rot_mat = { // * appears transposed because opengl column major memory ordering
        1,  0,  0,  0,
        0,  c,  s,  0,
        0, -s,  c,  0,
        0,  0,  0,  1
    };

    mat4_mul(out, *out, rot_mat);
}

void mat4_rotate_y(mat4* out, float a)
{
    float c = cosf(a);
    float s = sinf(a);
    mat4 rot_mat = { // * appears transposed because opengl column major memory ordering
        c,  0, -s,  0,
        0,  1,  0,  0,
        s,  0,  c,  0,
        0,  0,  0,  1 
    };

    mat4_mul(out, *out, rot_mat);
}

void mat4_rotate_z(mat4* out, float a)
{
    float c = cosf(a);
    float s = sinf(a);
    mat4 rot_mat = { // * appears transposed because opengl column major memory ordering
        c,  s,  0,  0,
        -s, c,  0,  0,
        0,  0,  1,  0,
        0,  0,  0,  1
    };

    mat4_mul(out, *out, rot_mat);
}

// calculate perspective matrix based on fov and aspect ratio
void mat_perspective_fov(mat4* out, float fov, float aspect, float near, float far)
{
    *out = mat4_zero();

    // calculating these beforehand is faster
    float s  = 1.0f / tanf(0.5f * fov); // 1 / half of the vertical fov
    float zdenom = 1.0f / (far - near);

    // matrix diagonal
    out->m11 = s / aspect; // aspect and tan on denominator
    out->m22 = s;
    out->m33 = -(far + near) * zdenom; // scaling z between 1 and -1 (once z/w)

    // not on diagonal
    out->m34 = (-2.0f * far * near) * zdenom; // scaling z between 1 and -1 (once z/w)
    out->m43 = -1.0f; // copy -z into w (flipped because opengl is in rh coords)
}

// calculate perspective matrix based on frustrum values
void mat_perspective_frustrum(mat4* out, float near, float far, float left, float right, float bottom, float top)
{
    *out = mat4_zero();

    // calculating these beforehand is faster
    float xdenom = 1.0f / (right - left);
    float ydenom = 1.0f / (top - bottom);
    float zdenom = 1.0f / (far - near);
    float near2 = 2.0f * near;

    // matrix diagonal
    out->m11 = near2 * xdenom;
    out->m22 = near2 * ydenom;
    out->m33 = -(far + near) * zdenom;
    //   m44

    // other z thing
    out->m34 = (-2.0f * far * near) * zdenom;

    // 3rd column
    out->m13 = (right + left) * xdenom;
    out->m23 = (top + bottom) * ydenom;
    //   m33
    out->m43 = -1.0f; 
}

void mat_orthographic_frustrum(mat4* out, float near, float far, float left, float right, float bottom, float top)
{
    *out = mat4_zero();

    // calculating these beforehand is faster
    float xdenom = 1.0f / (right - left);
    float ydenom = 1.0f / (top - bottom);
    float zdenom = 1.0f / (far - near);

    // matrix diagonal
    out->m11 = 2 * xdenom;
    out->m22 = 2 * ydenom;
    out->m33 = -2 * zdenom;
    out->m44 = 1.0f;

    // 4th column
    out->m14 = -(right + left) * xdenom;
    out->m24 = -(top + bottom) * ydenom;
    out->m34 = -(far + near) * zdenom;
    //   m44
}

// t is pos/translation from 0, k is dir or "z axis", i is right or "x axis"
// bit different from normal lookat matrix since it doesn't take in a target
void mat_look_at(mat4* out, vec3 t, vec3 k, vec3 i)
{
    vec3 j = vec_cross(i, k); // j is up or "y axis"
    vec3_norm(&j);

    /**
     * the inverse of the matrix which transforms you to the position/rotation of the player -> does the opposite so camera stays at center
     * k is negative because opengl rh coords
     * the product of a rotation and translation matrix
     * i, j and k represent the "axes" of the space where the player is
     */
    out->m11 = i.x; out->m12 = i.y; out->m13 = i.z; out->m14 = -vec3_dot(t, i);
    out->m21 = j.x; out->m22 = j.y; out->m23 = j.z; out->m24 = -vec3_dot(t, j);
    out->m31 = -k.x; out->m32 = -k.y; out->m33 = -k.z; out->m34 = vec3_dot(t, k);
    out->m41 = out->m42 = out->m43 = 0.0f; out->m44 = 1.0f;
}