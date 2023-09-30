#include "glmath.h"
#include <stdlib.h>
#include <string.h>

void vec3_copy(vec3 src, vec3* dest)
{
    dest->x = src.x;
    dest->y = src.y;
    dest->z = src.z;
}

vec3 vec3_zero()
{
    vec3 vec;
    vec.x = vec.y = vec.z = 0.0f;

    return vec;
}

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

vec3 vec3_norm(vec3 vec)
{
    vec3 new_vec;
    float denom = sqrtf(vec3_dot(vec, vec));

    if(denom == 0.0f) // prevent divide by zero
    {
        new_vec.x = new_vec.y = new_vec.z = 0.0f;
        return new_vec;
    }

    new_vec = vec3_scale(vec, 1.0f / denom);
    return new_vec;
}

vec3 vec_cross(vec3 v1, vec3 v2)
{
    vec3 new_vec;
    new_vec.x = v1.y * v2.z - v1.z * v2.y;
    new_vec.y = v1.z * v2.x - v1.x * v2.z;
    new_vec.z = v1.x * v2.y - v1.y * v2.x;

    return new_vec;
}

void vec2_copy(vec2 src, vec2* dest)
{
    dest->x = src.x;
    dest->y = src.y;
}

vec2 vec2_zero()
{
    vec2 vec;
    vec.x = vec.y = 0.0f;

    return vec;
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

vec2 vec2_norm(vec2 vec)
{
    vec2 new_vec;
    float denom = sqrtf(vec2_dot(vec, vec));

    if(denom == 0.0f) // prevent divide by zero
    {
        new_vec.x = new_vec.y = 0.0f;
        return new_vec;
    }

    new_vec = vec2_scale(vec, 1.0f / denom);
    return new_vec;
}

mat4 mat4_mul(mat4 m1, mat4 m2)
{
    mat4 new_mat;

    new_mat.m11 = m1.m11 * m2.m11 + m1.m12 * m2.m21 + m1.m13 * m2.m31 + m1.m14 * m2.m41;
    new_mat.m21 = m1.m21 * m2.m11 + m1.m22 * m2.m21 + m1.m23 * m2.m31 + m1.m24 * m2.m41;
    new_mat.m31 = m1.m31 * m2.m11 + m1.m32 * m2.m21 + m1.m33 * m2.m31 + m1.m34 * m2.m41;
    new_mat.m41 = m1.m41 * m2.m11 + m1.m42 * m2.m21 + m1.m43 * m2.m31 + m1.m44 * m2.m41;

    new_mat.m12 = m1.m11 * m2.m12 + m1.m12 * m2.m22 + m1.m13 * m2.m32 + m1.m14 * m2.m42;
    new_mat.m22 = m1.m21 * m2.m12 + m1.m22 * m2.m22 + m1.m23 * m2.m32 + m1.m24 * m2.m42;
    new_mat.m32 = m1.m31 * m2.m12 + m1.m32 * m2.m22 + m1.m33 * m2.m32 + m1.m34 * m2.m42;
    new_mat.m42 = m1.m41 * m2.m12 + m1.m42 * m2.m22 + m1.m43 * m2.m32 + m1.m44 * m2.m42;

    new_mat.m13 = m1.m11 * m2.m13 + m1.m12 * m2.m23 + m1.m13 * m2.m33 + m1.m14 * m2.m43;
    new_mat.m23 = m1.m21 * m2.m13 + m1.m22 * m2.m23 + m1.m23 * m2.m33 + m1.m24 * m2.m43;
    new_mat.m33 = m1.m31 * m2.m13 + m1.m32 * m2.m23 + m1.m33 * m2.m33 + m1.m34 * m2.m43;
    new_mat.m43 = m1.m41 * m2.m13 + m1.m42 * m2.m23 + m1.m43 * m2.m33 + m1.m44 * m2.m43;

    new_mat.m14 = m1.m11 * m2.m14 + m1.m12 * m2.m24 + m1.m13 * m2.m34 + m1.m14 * m2.m44;
    new_mat.m24 = m1.m21 * m2.m14 + m1.m22 * m2.m24 + m1.m23 * m2.m34 + m1.m24 * m2.m44;
    new_mat.m34 = m1.m31 * m2.m14 + m1.m32 * m2.m24 + m1.m33 * m2.m34 + m1.m34 * m2.m44;
    new_mat.m44 = m1.m41 * m2.m14 + m1.m42 * m2.m24 + m1.m43 * m2.m34 + m1.m44 * m2.m44;

    return new_mat;
}

mat4 mat4_zero()
{
    mat4 mat = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    return mat;
}

mat4 mat4_identity()
{
    mat4 mat =  {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 
    };

    return mat;
}

mat4 mat4_transpose(mat4 mat)
{
    mat4 new_mat;

    new_mat.m11 = mat.m11;
    new_mat.m21 = mat.m12;
    new_mat.m31 = mat.m13;
    new_mat.m41 = mat.m14;

    new_mat.m12 = mat.m21;
    new_mat.m22 = mat.m22;
    new_mat.m32 = mat.m23;
    new_mat.m42 = mat.m24;

    new_mat.m13 = mat.m31;
    new_mat.m23 = mat.m32;
    new_mat.m33 = mat.m33;
    new_mat.m43 = mat.m34;

    new_mat.m14 = mat.m41;
    new_mat.m24 = mat.m42;
    new_mat.m34 = mat.m43;
    new_mat.m44 = mat.m44;

    return new_mat;
}

mat4 mat4_scale(mat4 mat, vec3 s)
{
    mat4 scale_mat = {
        s.x, 0,   0,   0,
        0,   s.y, 0,   0,
        0,   0,   s.z, 0,
        0,   0,   0,   1
    };

    scale_mat = mat4_mul(mat, scale_mat);

    return scale_mat;
}

mat4 mat4_scale_scalar(mat4 mat, float s)
{
    // optimised method
    mat4 scaled_mat = { // appears transposed because opengl column major memory ordering
        s*mat.m11, s*mat.m21, s*mat.m31, s*mat.m41,
        s*mat.m12, s*mat.m22, s*mat.m32, s*mat.m42,
        s*mat.m13, s*mat.m23, s*mat.m33, s*mat.m43,
          mat.m14,   mat.m24,   mat.m34,   mat.m44
    };

    return scaled_mat;
}

mat4 mat4_trans(mat4 mat, vec3 t)
{
    mat4 trans_mat = { // appears transposed because opengl column major memory ordering
        1,   0,   0,   0,
        0,   1,   0,   0,
        0,   0,   1,   0,
        t.x, t.y, t.z, 1
    };

    trans_mat = mat4_mul(mat, trans_mat);
    
    return trans_mat;
}

mat4 mat4_rotate_x(mat4 mat, float a) // not figuring out arbitrary axis rotation
{
    float c = cosf(a);
    float s = sinf(a);
    mat4 rot_mat = { // appears transposed because opengl column major memory ordering
        1,  0,  0,  0,
        0,  c,  s,  0,
        0, -s,  c,  0,
        0,  0,  0,  1
    };

    rot_mat = mat4_mul(mat, rot_mat);

    return rot_mat;
}

mat4 mat4_rotate_y(mat4 mat, float a)
{
    float c = cosf(a);
    float s = sinf(a);
    mat4 rot_mat = { // appears transposed because opengl column major memory ordering
        c,  0, -s,  0,
        0,  1,  0,  0,
        s,  0,  c,  0,
        0,  0,  0,  1 
    };

    rot_mat = mat4_mul(mat, rot_mat);

    return rot_mat;
}

mat4 mat4_rotate_z(mat4 mat, float a)
{
    float c = cosf(a);
    float s = sinf(a);
    mat4 rot_mat = { // appears transposed because opengl column major memory ordering
        c,  s,  0,  0,
        -s, c,  0,  0,
        0,  0,  1,  0,
        0,  0,  0,  1
    };

    rot_mat = mat4_mul(mat, rot_mat);

    return rot_mat;
}

// calculate perspective matrix based on fov and aspect ratio
mat4 mat_perspective_fov(float fov, float aspect, float near, float far)
{
    mat4 mat = mat4_zero();

    // calculating these beforehand is faster
    float s  = 1.0f / tanf(0.5f * fov); // 1 / half of the vertical fov
    float zdenom = 1.0f / (far - near);

    // matrix diagonal
    mat.m11 = s / aspect; // aspect and tan on denominator
    mat.m22 = s;
    mat.m33 = -(far + near) * zdenom; // scaling z between 1 and -1 (once z/w)

    // not on diagonal
    mat.m34 = (-2.0f * far * near) * zdenom; // scaling z between 1 and -1 (once z/w)
    mat.m43 = -1.0f; // copy -z into w (flipped because opengl is in rh coords)

    return mat;
}

// calculate perspective matrix based on frustrum values
mat4 mat_perspective_frustrum(float near, float far, float left, float right, float bottom, float top)
{
    mat4 mat = mat4_zero();

    // calculating these beforehand is faster
    float xdenom = 1.0f / (right - left);
    float ydenom = 1.0f / (top - bottom);
    float zdenom = 1.0f / (far - near);
    float near2 = 2.0f * near;

    // matrix diagonal
    mat.m11 = near2 * xdenom;
    mat.m22 = near2 * ydenom;
    mat.m33 = -(far + near) * zdenom;

    // other z thing
    mat.m34 = (-2.0f * far * near) * zdenom;

    // 3rd column
    mat.m13 = (right + left) * xdenom;
    mat.m23 = (top + bottom) * ydenom;
    //  m33
    mat.m43 = -1.0f; 

    return mat;
}

mat4 mat_orthographic_frustrum(float near, float far, float left, float right, float bottom, float top)
{
    mat4 mat = mat4_zero();

    // calculating these beforehand is faster
    float xdenom = 1.0f / (right - left);
    float ydenom = 1.0f / (top - bottom);
    float zdenom = 1.0f / (far - near);

    // matrix diagonal
    mat.m11 = 2 * xdenom;
    mat.m22 = 2 * ydenom;
    mat.m33 = -2 * zdenom;
    mat.m44 = 1.0f;

    // 4th column
    mat.m14 = -(right + left) * xdenom;
    mat.m24 = -(top + bottom) * ydenom;
    mat.m34 = -(far + near) * zdenom;
    //  m44

    return mat;
}

// t is pos, k is dir or "z axis", i is right or "x axis"
// bit different from normal lookat matrix since it doesn't take in a target
mat4 mat_look_at(vec3 t, vec3 k, vec3 i)
{
    vec3 j = vec3_norm(vec_cross(i, k)); // j is up or "y axis"

    mat4 mat;

    // the inverse of the matrix which transforms you to the position/rotation of the player -> does the opposite so camera stays at center
    // k is negative because opengl rh coords
    // the product of a rotation and translation matrix
    // i, j and k represent the "axes" of the space where the player is
    mat.m11 = i.x; mat.m12 = i.y; mat.m13 = i.z; mat.m14 = -vec3_dot(t, i);
    mat.m21 = j.x; mat.m22 = j.y; mat.m23 = j.z; mat.m24 = -vec3_dot(t, j);
    mat.m31 = -k.x; mat.m32 = -k.y; mat.m33 = -k.z; mat.m34 = vec3_dot(t, k);
    mat.m41 = mat.m42 = mat.m43 = 0.0f; mat.m44 = 1.0f;

    return mat;
}