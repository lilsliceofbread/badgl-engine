#ifndef BADGL_SCENE_H
#define BADGL_SCENE_H

#include <inttypes.h>
#include <stdbool.h>
#include "camera.h"
#include "model.h"
#include "skybox.h"
#include "arena_alloc.h"
#include "glmath.h"
#include "light.h"

typedef void (*SceneUpdateFunc)();

#define MAX_LIGHTS 32
#define MODEL_ALLOC_SIZE 4

typedef enum SceneFlags {
    HAS_SKYBOX = 1 << 0,
} SceneFlags;

typedef struct Scene {
    Camera cam;

    Model* models;
    uint32_t model_count;
    Model skybox;

    Light lights[MAX_LIGHTS];
    int32_t light_count;
    UBO light_ubo;
    DirLight dir_light;

    SceneUpdateFunc user_update_func; // user func must take 1 param, a Scene*

    SceneFlags flags;
} Scene;

// user func must take 1 param, a Scene*
void scene_init(Scene* self, vec3 start_pos, vec2 euler, const DirLight* dir_light,
                Renderer* rd, const char* skybox_cubemap_path, SceneUpdateFunc func);

void scene_reallocate_models(Scene* self, uint32_t new_count);

uint32_t scene_add_model(Scene* self, const Model* model);

void scene_add_light(Scene* self, Renderer* rd, const Light* light, const Model* model);

void scene_set_dir_light(Scene* self, const DirLight* light);

// call this when changing lights
void scene_update_lights(Scene* self, Renderer* rd);

// this needs to be called when switching to a new scene
void scene_switch(Scene* self, Renderer* rd);

void scene_update(Scene* self, Renderer* rd);

void scene_draw(Scene* self, Renderer* rd);

void scene_free(Scene* self);

#endif