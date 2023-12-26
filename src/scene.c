#include "scene.h"

#define DEFAULT_FOV 90.0f
#define DEFAULT_ZNEAR 0.01f
#define DEFAULT_ZFAR 100.0f

void scene_init(Scene* self, vec3 start_pos, vec2 euler, Renderer* rd, const char* skybox_cubemap_path)
{
    const float aspect_ratio = (float)(rd->width) / (float)(rd->height); // not casting these to float causes the aspect ratio to be rounded
    self->cam = camera_init(start_pos, euler.y, euler.x); // yaw then pitch
    camera_update_proj(&self->cam, DEFAULT_FOV, aspect_ratio, DEFAULT_ZNEAR, DEFAULT_ZFAR);

    self->models = NULL;
    self->model_count = 0;

    if(skybox_cubemap_path == NULL)
    {
        self->has_skybox = false;
        return;
    }

    self->skybox = skybox_init(skybox_cubemap_path);
    self->has_skybox = true;
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
    self->models[self->model_count++] = *model; // i think this is better than just passing by value
    return;
}

void scene_update(Scene* self, Renderer* rd)
{
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

    skybox_free(&self->skybox);
}