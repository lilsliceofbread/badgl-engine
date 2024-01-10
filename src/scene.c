#include "scene.h"

#include "defines.h"

#define DEFAULT_FOV 90.0f
#define DEFAULT_ZNEAR 0.01f
#define DEFAULT_ZFAR 100.0f

#define GLSL_LIGHT_SIZE 80
#define GLSL_INT_SIZE 4

void scene_init(Scene* self, vec3 start_pos, vec2 euler, const DirLight* dir_light,
                Renderer* rd, const char* skybox_cubemap_path, SceneUpdateFunc func)
{
    const float aspect_ratio = (float)(rd->width) / (float)(rd->height); // not casting these to float causes the aspect ratio to be rounded
    self->cam = camera_init(start_pos, euler.y, euler.x); // yaw then pitch
    camera_update_proj(&self->cam, DEFAULT_FOV, aspect_ratio, DEFAULT_ZNEAR, DEFAULT_ZFAR);

    self->models = NULL;
    self->model_count = 0;
    self->light_count = 0;
    self->user_update_func = func;
    self->flags = 0;

    if(skybox_cubemap_path != NULL)
    {
        self->skybox = skybox_init(rd, skybox_cubemap_path);
        self->flags |= HAS_SKYBOX;
    }

    if(dir_light != NULL) self->dir_light = *dir_light;

    ASSERT(sizeof(Light) == GLSL_LIGHT_SIZE, "SCENE: platform struct packing/padding does not match GLSL. wtf?\n");

    self->light_ubo = ubo_create();

    size_t light_ubo_size = MAX_LIGHTS * sizeof(Light) + GLSL_INT_SIZE;
    ubo_bind(self->light_ubo);
    ubo_set_buffer(self->light_ubo, NULL, light_ubo_size, true); // configure buffer size
    ubo_unbind(self->light_ubo);
}

void scene_reallocate_models(Scene* self, uint32_t new_count)
{
    uint32_t model_array_size = ALIGNED_SIZE(self->model_count, MODEL_ALLOC_SIZE);
    if(new_count > model_array_size)
    {
        uint32_t new_array_size = ALIGNED_SIZE(new_count, MODEL_ALLOC_SIZE);

        self->models = (Model*)realloc(self->models, new_array_size * sizeof(Model));
        ASSERT(self->models != NULL, "SCENE: models reallocation failed");
        BADGL_LOG("SCENE: models array resize from %u to %u\n", model_array_size, new_array_size);
    }
}

// add string associated with models instead of using indexes?
uint32_t scene_add_model(Scene* self, const Model* model)
{
    scene_reallocate_models(self, self->model_count + 1);
    self->models[self->model_count++] = *model;

    return self->model_count - 1;
}

void scene_add_light(Scene* self, Renderer* rd, const Light* light, const Model* model)
{
    if(light == NULL) return;
    if(self->light_count + 1 > MAX_LIGHTS)
    {
        BADGL_LOG("SCENE: cannot add light; max lights reached\n");
        return;
    }

    self->lights[self->light_count++] = *light;
    if(model != NULL)
    {
        uint32_t model_idx = scene_add_model(self, model);
        Model* curr_model = &self->models[model_idx];

        curr_model->shader_idx = rd->light_shader; // enforce shader as light shader

        curr_model->material.flags |= IS_LIGHT;
        curr_model->material.ambient = VEC4TOVEC3(light->ambient); 
        curr_model->material.diffuse = VEC4TOVEC3(light->diffuse); 
        curr_model->material.specular = VEC4TOVEC3(light->specular); 

        Transform transform = {
            .pos = VEC4TOVEC3(light->pos),
            .euler = VEC3(0.0f, 0.0f, 0.0f),
            .scale = VEC3(1.0f, 1.0f, 1.0f)
        };
        model_update_transform(curr_model, &transform);
    }

    ubo_bind(self->light_ubo);
    ubo_set_buffer_region(self->light_ubo, self->lights, 0, MAX_LIGHTS * sizeof(Light));
    ubo_set_buffer_region(self->light_ubo, &self->light_count, MAX_LIGHTS * sizeof(Light), GLSL_INT_SIZE);
    ubo_unbind(self->light_ubo);
}

void scene_set_dir_light(Scene* self, const DirLight* light)
{
    if(light == NULL) return;

    self->dir_light = *light;
}

void scene_update_lights(Scene* self, Renderer* rd)
{
    ubo_bind_buffer_range(self->light_ubo, 0, 0, MAX_LIGHTS * sizeof(Light) + GLSL_INT_SIZE); // hardcoded for now

    uint32_t* shaders = malloc(self->model_count * sizeof(uint32_t)); // ensure enough space in worst case
    uint32_t shader_count = 0;
    for(uint32_t i = 0; i < self->model_count; i++)
    {
        const Model* curr_model = &self->models[i];
        if(curr_model->material.flags & (NO_LIGHTING | IS_LIGHT)) continue;

        uint32_t curr_shader = curr_model->shader_idx;
        for(uint32_t i = 0; i < shader_count; i++)
        {
            if(curr_shader == shaders[i]) continue;
        }

        shaders[shader_count++] = curr_shader;
    }

    for(uint32_t i = 0; i < shader_count; i++)
    {
        Shader* curr_shader = &rd->shaders[shaders[i]];

        shader_use(curr_shader);
        dir_light_set_uniforms(&self->dir_light, curr_shader);
    }

    free(shaders);
}

void scene_switch(Scene* self, Renderer* rd)
{
    scene_update_lights(self, rd);
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

    if(self->flags & HAS_SKYBOX) skybox_draw(&self->skybox, rd, &self->cam); // drawn last after depth buffer filled
}

void scene_free(Scene* self)
{
    skybox_free(&self->skybox);
    for(uint32_t i = 0; i < self->model_count; i++)
    {
        model_free(&self->models[i]);
    }
    if(self->models != NULL) free(self->models); // in case no models were added

    ubo_free(self->light_ubo);
}