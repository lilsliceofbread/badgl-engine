#ifndef BADGL_SCENE_H
#define BADGL_SCENE_H

#include <inttypes.h>
#include <stdbool.h>
#include "camera.h"
#include "model.h"
#include "skybox.h"
#include "arena_alloc.h"
#include "glmath.h"

typedef void (*SceneUpdateFunc)();

typedef struct Scene {
    Camera cam;

    Model skybox;
    bool has_skybox;

    Model* models;
    uint32_t model_count;

    SceneUpdateFunc user_update_func; // user func must take 1 param, a Scene*
} Scene;

// user func must take 1 param, a Scene*
void scene_init(Scene* self, vec3 start_pos, vec2 euler, Renderer* rd, const char* skybox_cubemap_path, SceneUpdateFunc func);

void scene_add_model(Scene* self, Model* model);

void scene_update(Scene* self, Renderer* rd);

void scene_draw(Scene* self, Renderer* rd);

void scene_free(Scene* self);

#endif