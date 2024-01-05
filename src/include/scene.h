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

typedef struct Scene {
    Camera cam;

    Model skybox;
    bool has_skybox;

    Model* models;
    uint32_t model_count;

    SceneUpdateFunc user_update_func; // user func must take 1 param, a Scene*

    Light lights[MAX_LIGHTS];
    int32_t light_count;
    UBO light_ubo;
} Scene;

// user func must take 1 param, a Scene*
void scene_init(Scene* self, vec3 start_pos, vec2 euler, Renderer* rd, const char* skybox_cubemap_path, SceneUpdateFunc func);

void scene_add_model(Scene* self, Model* model);

void scene_add_light(Scene* self, Light* light);

/*  this needs to be called when switching to a new scene
    sets up the values which may have been messed up by other scenes  */
void scene_switch(Scene* self);

void scene_update(Scene* self, Renderer* rd);

void scene_draw(Scene* self, Renderer* rd);

void scene_free(Scene* self);

#endif