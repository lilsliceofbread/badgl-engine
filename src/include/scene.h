#ifndef BADGL_SCENE_H
#define BADGL_SCENE_H

#include <inttypes.h>
#include <stdbool.h>
#include "camera.h"
#include "model.h"
#include "skybox.h"
#include "arena_alloc.h"
#include "glmath.h"

typedef struct Scene {
    Camera cam;
    mat4 vp;

    Model skybox;
    bool has_skybox;

    Model* models;
    uint32_t model_count;
} Scene;

void scene_init(Scene* self, vec3 start_pos, vec2 euler, Renderer* rd, const char* skybox_cubemap_path);

void scene_add_model(Scene* self, Model* model);

void scene_update(Scene* self, Renderer* rd);

void scene_draw(Scene* self, Renderer* rd);

void scene_free(Scene* self);

#endif