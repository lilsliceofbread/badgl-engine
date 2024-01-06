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

typedef enum SceneFlags {
    HAS_SKYBOX = 1 << 0,
    HAS_DIR_LIGHT = 1 << 1
} SceneFlags;

typedef struct Scene {
    Camera cam;

    Model* models;
    uint32_t model_count;
    Model skybox;

    Light lights[MAX_LIGHTS];
    //uint32_t light_model_indexes[MAX_LIGHTS]; // use the light's index to index into this to index into models array
    int32_t light_count;
    UBO light_ubo;
    DirLight dir_light;

    SceneUpdateFunc user_update_func; // user func must take 1 param, a Scene*

    SceneFlags flags;
} Scene;

// user func must take 1 param, a Scene*
void scene_init(Scene* self, vec3 start_pos, vec2 euler, const DirLight* dir_light,
                Renderer* rd, const char* skybox_cubemap_path, SceneUpdateFunc func);

Model* scene_add_model(Scene* self, const Model* model);

void scene_add_light(Scene* self, Renderer* rd, const Light* light, const Model* model);

void scene_set_dir_light(Scene* self, const DirLight* light);

// this needs to be called when switching to a new scene or changing lights while rendering
void scene_update_lights(Scene* self);

void scene_update(Scene* self, Renderer* rd);

void scene_draw(Scene* self);

void scene_free(Scene* self);

#endif