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

#define GLSL_MAX_LIGHTS 32
#define SCENE_MODEL_ALLOC_SIZE 4

typedef enum SceneFlags {
    HAS_SKYBOX = 1 << 0,
} SceneFlags;

typedef struct Scene {
    Camera cam;

    Model* models;
    uint32_t model_count;
    Model skybox;

    Light lights[GLSL_MAX_LIGHTS];
    int32_t light_count;
    UBO light_ubo;
    DirLight dir_light;

    SceneUpdateFunc user_update_func;

    SceneFlags flags;
} Scene;

/**
 * @brief initialise scene
 * @note this function must any other scene function is used with your scene
 * @param self:
 * @param start_pos initial position of the camera
 * @param euler initial rotation of the camera (x: pitch, y: yaw)
 * @param dir_light optional directional light for the scene (pass NULL to ignore)
 * @param rd 
 * @param skybox_cubemap_path optional path to a skybox (to format, see texture_cubemap_create) (pass NULL to ignore)
 * @param func optional user update callback to run every scene_update (pass NULL to ignore)
 * @retval None
*/
void scene_init(Scene* self, vec3 start_pos, vec2 start_euler, const DirLight* dir_light,
                Renderer* rd, const char* skybox_cubemap_path, SceneUpdateFunc func);

/**
 * ! this function is for library use only
 * ? should this function be exposed to users? inline?
 * @brief if model array is out of space, reallocate aligned with block size 
 * @note   
 * @param  self: 
 * @param  new_count: the new amount of models
 * @retval None
 */
void scene_reallocate_models(Scene* self, uint32_t new_count);

/**
 * @brief add a model to your scene 
 * @note the pointer to your model is not modified; the data is copied
 * @param  self: 
 * @param  model: const pointer to your model
 * @retval the index of the model in scene->models
 */
uint32_t scene_add_model(Scene* self, const Model* model);

/**
 * @brief add a point light to the scene 
 * @note  pointers to your light or model are not modified; the data is copied 
 * @param  self: 
 * @param  rd: 
 * @param  light: your light 
 * @param  model: an optional model for the light (pass NULL to be invisible)
 * @retval None
 */
void scene_add_light(Scene* self, Renderer* rd, const Light* light, const Model* model);

void scene_set_dir_light(Scene* self, const DirLight* light);

void scene_update_lights(Scene* self, Renderer* rd);

void scene_switch(Scene* self, Renderer* rd);

void scene_update(Scene* self, Renderer* rd);

void scene_draw(Scene* self, Renderer* rd);

void scene_free(Scene* self);

#endif