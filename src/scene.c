#include "scene.h"

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

    if(dir_light != NULL)
    {
        self->dir_light = *dir_light;
        self->flags |= HAS_DIR_LIGHT;
    }

    ASSERT(sizeof(Light) == GLSL_LIGHT_SIZE, "SCENE: platform struct packing/padding does not match GLSL. wtf?\n");

    self->light_ubo = ubo_create();

    size_t light_ubo_size = MAX_LIGHTS * sizeof(Light) + GLSL_INT_SIZE;
    ubo_bind(self->light_ubo);
    ubo_set_buffer(self->light_ubo, NULL, light_ubo_size, false); // configure buffer size
    ubo_unbind(self->light_ubo);
}

// add string associated with models instead of using indexes?
Model* scene_add_model(Scene* self, const Model* model)
{
    self->models = (Model*)realloc(self->models, (self->model_count + 1) * sizeof(Model));
    ASSERT(self->models != NULL, "SCENE: failed to reallocate model array");
    self->models[self->model_count++] = *model;

    return &self->models[self->model_count - 1];
}

void scene_add_light(Scene* self, Renderer* rd, const Light* light, const Model* model)
{
    if(light == NULL) return;
    if(self->light_count + 1 > MAX_LIGHTS)
    {
        printf("SCENE: cannot add light; max lights reached\n");
        return;
    }

    self->lights[self->light_count++] = *light;
    if(model != NULL)
    {
        Model* curr_model = scene_add_model(self, model);
        curr_model->shader = &rd->light_shader; // enforce shader as light shader
        
        curr_model->material.flags |= IS_LIGHT;                 // set material to match light
        curr_model->material.ambient = VEC4TO3(light->ambient); 
        curr_model->material.diffuse = VEC4TO3(light->diffuse); 
        curr_model->material.specular = VEC4TO3(light->specular); 

        Transform transform = {
            .pos = VEC4TO3(light->pos),
            .euler = (vec3){0.0f, 0.0f, 0.0f},
            .scale = (vec3){1.0f, 1.0f, 1.0f}
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
    self->flags |= HAS_DIR_LIGHT;
}

void scene_update_lights(Scene* self)
{
    ubo_bind_buffer_range(self->light_ubo, 0, 0, MAX_LIGHTS * sizeof(Light) + GLSL_INT_SIZE); // hardcoded for now

    if(!(self->flags & HAS_DIR_LIGHT)) return;

    Shader* shaders[64]; // should be fine (hopefully)
    uint32_t shader_count = 0;
    for(uint32_t i = 0; i < self->model_count; i++)
    {
        const Model* curr_model = &self->models[i];
        if(curr_model->material.flags & NO_LIGHTING || curr_model->material.flags & IS_LIGHT) continue;

        Shader* curr_shader = curr_model->shader;
        for(uint32_t i = 0; i < shader_count; i++)
        {
            if(curr_shader == shaders[i]) continue;
        }

        shaders[shader_count++] = curr_shader;
    }

    for(uint32_t i = 0; i < shader_count; i++)
    {
        Shader* curr_shader = shaders[i];

        shader_use(curr_shader);
        dir_light_set_uniforms(&self->dir_light, curr_shader);
    }
}

void scene_update(Scene* self, Renderer* rd)
{
    if(self->user_update_func != NULL) self->user_update_func(self);

    camera_update(&self->cam, rd);
}

void scene_draw(Scene* self)
{
    for(uint32_t i = 0; i < self->model_count; i++)
    {
        model_draw(&self->models[i], &self->cam);
    }

    if(self->flags & HAS_SKYBOX) skybox_draw(&self->skybox, &self->cam); // drawn last after depth buffer filled
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