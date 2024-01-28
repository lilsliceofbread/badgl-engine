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

#define GLSL_MAX_LIGHTS 32 // * at some point create polyglot include for C and GLSL
#define SCENE_MODEL_ALLOC_SIZE 4

typedef enum SceneFlags {
    HAS_SKYBOX = 1 << 0,
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
 * @brief initialise scene
 * @note this function must any other scene function is used with your scene
 * @param self
 * @param rd 
 * @param start_pos initial position of the camera
 * @param start_euler initial rotation of the camera (x: pitch, y: yaw)
 * @retval None
*/
void scene_create(Scene* self, Renderer* rd, vec3 start_pos, vec2 start_euler);

void scene_set_skybox(Scene* self, Renderer* rd, const char* cubemap_path);

void scene_set_update_callback(Scene* self, SceneUpdateFunc func);

/**
 * @brief add a model to your scene 
 * @note the pointer to your model is not modified; the data is copied
 * @param  self: 
 * @param  model: const pointer to your model
 * @retval the index of the model in scene->models
 */
u32 scene_add_model(Scene* self, const Model* model);

/**
 * @brief add a point light to the scene 
 * @note  pointers to your light or model are not modified; the data is copied 
 * @param  self: 
 * @param  rd: 
 * @param  light: your light 
 * @param  model: an optional model that is aligned with the light and has the same material (pass NULL to use default sphere)
 * @retval None
 */
void scene_add_light(Scene* self, Renderer* rd, const Light* light, const Model* model);

void scene_set_dir_light(Scene* self, const DirLight* light);

void scene_update_lights(Scene* self, Renderer* rd);

// call this to update light data without rendering it to the screen
// use this if you want to update light data for another scene while
// a different one is being rendered, so as to prevent disturbing that scene's graphics
void scene_update_light_data(Scene* self);

void scene_switch(Scene* self, Renderer* rd);

void scene_update(Scene* self, Renderer* rd);

void scene_draw(Scene* self, Renderer* rd);

void scene_free(Scene* self);

#endif