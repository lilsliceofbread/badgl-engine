#ifndef BGL_SCENE_H
#define BGL_SCENE_H

#include "defines.h"
#include "camera.h"
#include "model.h"
#include "skybox.h"
#include "arena_alloc.h"
#include "glmath.h"
#include "light.h"

typedef void (*SceneUpdateFunc)();

#define GLSL_MAX_LIGHTS 32

typedef enum SceneFlags {
    BGL_SCENE_HAS_SKYBOX = 1 << 0,
} SceneFlags;

typedef struct Scene {
    Camera cam;

    Model* models;
    u32 model_count;
    Model skybox;

    Light lights[GLSL_MAX_LIGHTS];
    u32 light_models[GLSL_MAX_LIGHTS];
    i32 light_count;
    UBO light_ubo;
    DirLight dir_light;

    SceneUpdateFunc user_update_func;

    SceneFlags flags;
} Scene;

/**
 * @param  start_pos: initial position of the camera
 * @param  start_euler: initial rotation of the camera (x: pitch, y: yaw)
*/
void scene_create(Scene* self, Renderer* rd, vec3 start_pos, vec2 start_euler);

/**
 * @param  cubemap_path: path to skybox cubemap (for more info about format of path and cubemap read texture_cubemap_create)
 */
void scene_set_skybox(Scene* self, Renderer* rd, const char* cubemap_path);

/**
 * @param  func: the function to call in scene_update (this function must take only one parameter, a Scene*)
 */
void scene_set_update_callback(Scene* self, SceneUpdateFunc func);

/**
 * @returns the index of the model in scene->models
 */
u32 scene_add_model(Scene* self, const Model* model);

/**
 * @param  model: an optional model that is aligned with the light and has the same material (pass NULL to use default sphere)
 */
void scene_add_light(Scene* self, Renderer* rd, const Light* light, const Model* model);

void scene_set_dir_light(Scene* self, const DirLight* light);

/**
 * @brief  a function which updates the light data and syncs it with the GPU
 * @note   call this when you have added or changed some of the lights
 */
void scene_update_lights(Scene* self, Renderer* rd);

/**
 * @brief  update light data without rendering it to the screen
 * @note   use this if you want to update light data for another scene while a different one is being rendered, so as to prevent disturbing that scene's graphics
 */
void scene_update_light_data(Scene* self);

/**
 * @brief  resets data that may have been changed when rendering another scene e.g. lights
 * @note   call this when switching to a new scene
 */
void scene_switch(Scene* self, Renderer* rd);

void scene_update(Scene* self, Renderer* rd);

void scene_draw(Scene* self, Renderer* rd);

void scene_free(Scene* self);

#endif
