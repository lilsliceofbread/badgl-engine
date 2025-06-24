#include "scene.h"

#include "defines.h"
#include "shapes.h"
#include "model.h"
#include "defines.glsl" // constants shared between c and glsl

#define DEFAULT_FOV 90.0f
#define DEFAULT_ZNEAR 0.01f
#define DEFAULT_ZFAR 100.0f

#define MOVESPEED 5.0f // hardcoded for now
#define SENSITIVITY 0.1f

/**
 * internal functions
 */
void scene_update_light_model(Scene* self, u32 index);
void scene_send_lights(Scene* self, Renderer* rd);

void scene_create(Scene* self, Renderer* rd, vec3 start_pos, vec2 start_euler)
{
    const f32 aspect_ratio = (f32)(rd->width) / (f32)(rd->height); // not casting these to f32 causes the aspect ratio to be rounded
    self->cam = camera_create(start_pos, start_euler.x, start_euler.y, MOVESPEED, SENSITIVITY);
    camera_update_proj(&self->cam, DEFAULT_FOV, aspect_ratio, DEFAULT_ZNEAR, DEFAULT_ZFAR);

    self->models = NULL;
    self->model_count = 0;
    self->light_count = 0;
    self->flags = 0;
    self->user_update_func = NULL;

    self->dir_light.dir = VEC3(0.0f, 0.0f, 0.0f);
    self->dir_light.ambient = VEC3(0.0f, 0.0f, 0.0f);
    self->dir_light.diffuse = VEC3(0.0f, 0.0f, 0.0f);
    self->dir_light.specular = VEC3(0.0f, 0.0f, 0.0f);

    if(rd->flags & BGL_RD_LIGHTING_OFF) return;

    self->light_ubo = ubo_create();

    u32 light_ubo_size = BGL_GLSL_MAX_LIGHTS * sizeof(Light) + BGL_GLSL_INT_SIZE;
    ubo_bind(self->light_ubo);
    ubo_set_buffer(self->light_ubo, NULL, light_ubo_size, true); // configure buffer size
    ubo_unbind(self->light_ubo);
}

void scene_set_skybox(Scene* self, Renderer* rd, const char* cubemap_path)
{
    if(cubemap_path != NULL && !(rd->flags & BGL_RD_SKYBOX_OFF))
    {
        self->skybox = skybox_create(rd, cubemap_path);
        self->flags |= BGL_SCENE_HAS_SKYBOX;
    }
}

void scene_set_update_callback(Scene* self, SceneUpdateFunc func)
{
    self->user_update_func = func;
}

// TODO: add string associated with models instead of using indices?
u32 scene_add_model(Scene* self, const Model* model)
{
    BLOCK_RESIZE_ARRAY(&self->models, Model, self->model_count, 1);
    self->models[self->model_count++] = *model;

    return self->model_count - 1;
}

void scene_add_light(Scene* self, Renderer* rd, const Light* light, const Model* model)
{
    if(rd->flags & BGL_RD_LIGHTING_OFF) return;

    BGL_ASSERT(light != NULL, "provided light is null");
    if(self->light_count >= BGL_GLSL_MAX_LIGHTS)
    {
        BGL_LOG_WARN("cannot add light to scene; max lights reached");
        return;
    }

    u32 model_idx;
    if(model != NULL)
    {
        model_idx = scene_add_model(self, model);
    }
    else
    {
        Model sphere;
        Transform transform;

        shapes_uv_sphere(&sphere, 15, NULL, 0);

        transform_reset(&transform);
        transform.scale = VEC3(0.3f, 0.3f, 0.3f);
        model_update_transform(&sphere, &transform);

        model_idx = scene_add_model(self, &sphere);
    }

    self->lights[self->light_count] = *light;
    self->light_models[self->light_count] = model_idx;
    self->light_count++;

    self->models[model_idx].shader_idx = rd->light_shader; // enforce shader as light shader
    self->models[model_idx].material.flags |= BGL_MATERIAL_IS_LIGHT;
}

void scene_set_dir_light(Scene* self, const DirLight* light)
{
    BGL_ASSERT(light != NULL, "provided dir_light is null");

    self->dir_light = *light;
}

void scene_update_lights(Scene* self, Renderer* rd)
{
    if(rd->flags & BGL_RD_LIGHTING_OFF) return;

    scene_update_light_data(self);
    scene_send_lights(self, rd);
}

void scene_switch(Scene* self, Renderer* rd)
{
    if(rd->flags & BGL_RD_LIGHTING_OFF) return;

    scene_send_lights(self, rd);
}

void scene_update(Scene* self, Renderer* rd)
{
    if(self->user_update_func != NULL) self->user_update_func(self);

    camera_update(&self->cam, rd);
}

void scene_draw(Scene* self, Renderer* rd)
{
    for(u32 i = 0; i < self->model_count; i++)
    {
        model_draw(&self->models[i], rd, &self->cam);
    }

    if(self->flags & BGL_SCENE_HAS_SKYBOX) skybox_draw(&self->skybox, rd, &self->cam); // drawn last after depth buffer filled
}

void scene_free(Scene* self)
{
    skybox_free(&self->skybox);
    for(u32 i = 0; i < self->model_count; i++)
    {
        model_free(&self->models[i]);
    }
    if(self->models != NULL) BGL_FREE(self->models); // in case no models were added

    ubo_free(self->light_ubo);
}

void scene_update_light_model(Scene* self, u32 index)
{
    BGL_ASSERT(index < (u32)self->light_count, "light index exceeds end of light buffer");

    Light* light = &self->lights[index];
    Model* light_model = &self->models[self->light_models[index]];
    Transform transform = light_model->transform;

    light_model->material.ambient = VEC4TOVEC3(light->ambient); 
    light_model->material.diffuse = VEC4TOVEC3(light->diffuse); 
    light_model->material.specular = VEC4TOVEC3(light->specular); 

    transform.pos = VEC4TOVEC3(light->pos); // TODO: make this optional/move light relative to model?
    model_update_transform(light_model, &transform);
}

void scene_update_light_data(Scene* self)
{
    const u32 light_buf_size = BGL_GLSL_MAX_LIGHTS * sizeof(Light);
    ubo_bind(self->light_ubo);
    ubo_set_buffer_region(self->light_ubo, self->lights,       0,                   light_buf_size);
    ubo_set_buffer_region(self->light_ubo, &self->light_count, (i32)light_buf_size, BGL_GLSL_INT_SIZE);
    ubo_unbind(self->light_ubo);

    for(i32 i = 0; i < self->light_count; i++)
    {
        scene_update_light_model(self, (u32)i);
    }
}

void scene_send_lights(Scene* self, Renderer* rd)
{
    if(rd->flags & BGL_RD_LIGHTING_OFF) return;

    ubo_bind_buffer_range(self->light_ubo, 0, 0, BGL_GLSL_MAX_LIGHTS * sizeof(Light) + BGL_GLSL_INT_SIZE); // TODO: remove hardcoding of location/index

    // TODO: add dir_light to ubo so as to not do this garbage (the light which has pos of 0, 0, 0)
    /* updating dir_light for all shaders */

    u32 shaders[self->model_count];
    u32 shader_count = 0;
    for(u32 i = 0; i < self->model_count; i++)
    {
        const Model* curr_model = &self->models[i];
        if(curr_model->material.flags & (BGL_MATERIAL_NO_LIGHTING | BGL_MATERIAL_IS_LIGHT)) continue;

        u32 curr_shader = curr_model->shader_idx;
        for(u32 i = 0; i < shader_count; i++)
        {
            if(curr_shader == shaders[i]) continue;
        }

        shaders[shader_count++] = curr_shader;
    }

    for(u32 i = 0; i < shader_count; i++)
    {
        Shader* curr_shader = &rd->shaders[shaders[i]];

        shader_use(curr_shader);
        dir_light_set_uniforms(&self->dir_light, curr_shader);
    }
}
