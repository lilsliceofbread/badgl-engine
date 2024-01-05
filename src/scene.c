#include "scene.h"

#define DEFAULT_FOV 90.0f
#define DEFAULT_ZNEAR 0.01f
#define DEFAULT_ZFAR 100.0f

#define GLSL_LIGHT_SIZE 80
#define GLSL_INT_SIZE 4

void scene_init(Scene* self, vec3 start_pos, vec2 euler, Renderer* rd, const char* skybox_cubemap_path, SceneUpdateFunc func)
{
    const float aspect_ratio = (float)(rd->width) / (float)(rd->height); // not casting these to float causes the aspect ratio to be rounded
    self->cam = camera_init(start_pos, euler.y, euler.x); // yaw then pitch
    camera_update_proj(&self->cam, DEFAULT_FOV, aspect_ratio, DEFAULT_ZNEAR, DEFAULT_ZFAR);

    self->models = NULL;
    self->model_count = 0;
    self->light_count = 0;
    self->user_update_func = func;

    if(skybox_cubemap_path == NULL)
    {
        self->has_skybox = false;
        return;
    }

    self->skybox = skybox_init(skybox_cubemap_path);
    self->has_skybox = true;

    ASSERT(sizeof(Light) == GLSL_LIGHT_SIZE, "SCENE: platform struct packing/padding does not match GLSL. wtf?\n");

    self->light_ubo = ubo_create();

    size_t light_ubo_size = MAX_LIGHTS * sizeof(Light) + GLSL_INT_SIZE;
    ubo_bind(self->light_ubo);
    ubo_set_buffer(self->light_ubo, NULL, light_ubo_size, false); // configure buffer size
    ubo_unbind(self->light_ubo);
}

// add string associated with models instead of using indexes?
void scene_add_model(Scene* self, Model* model)
{
    if(self->model_count == 0)
    {
        self->models = (Model*)malloc(sizeof(Model));
        ASSERT(self->models != NULL, "SCENE: failed to allocate model array");
        self->models[self->model_count++] = *model;
        return;
    }

    self->models = (Model*)realloc(self->models, (self->model_count + 1) * sizeof(Model));
    ASSERT(self->models != NULL, "SCENE: failed to reallocate model array");
    self->models[self->model_count++] = *model;
    return;
}

void scene_add_light(Scene* self, Light* light)
{
    if(self->light_count + 1 > MAX_LIGHTS)
    {
        printf("SCENE: cannot add light; max lights reached\n");
        return;
    }

    self->lights[self->light_count++] = *light;

    ubo_bind(self->light_ubo);
    ubo_set_buffer_region(self->light_ubo, self->lights, 0, MAX_LIGHTS * sizeof(Light));
    ubo_set_buffer_region(self->light_ubo, &self->light_count, MAX_LIGHTS * sizeof(Light), GLSL_INT_SIZE);
    ubo_unbind(self->light_ubo);
}

void scene_switch(Scene* self)
{
    ubo_bind_buffer_range(self->light_ubo, 0, 0, MAX_LIGHTS * sizeof(Light) + GLSL_INT_SIZE); // hardcoded for now
}

void scene_update(Scene* self, Renderer* rd)
{
    if(self->user_update_func != NULL) self->user_update_func(self);

    camera_update(&self->cam, rd);
}

void scene_draw(Scene* self, Renderer* rd)
{
    for(uint32_t i = 0; i < self->model_count; i++)
    {
        model_draw(&self->models[i], rd, &self->cam);
    }

    if(self->has_skybox) skybox_draw(&self->skybox, rd, &self->cam); // must be drawn last after everything else has filled the depth buffer
}

void scene_free(Scene* self)
{
    for(uint32_t i = 0; i < self->model_count; i++)
    {
        model_free(&self->models[i]);
    }
    free(self->models);

    skybox_free(&self->skybox);
}