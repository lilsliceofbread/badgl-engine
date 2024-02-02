#ifndef BGL_TRANSFORM_H
#define BGL_TRANSFORM_H

typedef struct Transform {
    vec3 pos;
    vec3 euler;
    vec3 scale;
} Transform;

/**
 * @brief  reset transform to default values
 */
static inline void transform_reset(Transform* transform)
{
    transform->pos = VEC3(0.0f, 0.0f, 0.0f);
    transform->euler = VEC3(0.0f, 0.0f, 0.0f);
    transform->scale = VEC3(1.0f, 1.0f, 1.0f);
}

#endif