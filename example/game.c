#include "game.h"

#include <stdio.h>
#include <stdbool.h>
#include "glmath.h"
#include "renderer.h"
#include "camera.h"
#include "util.h"
#include "model.h"

#define GAME_WIDTH 1280
#define GAME_HEIGHT 720

void game_init(GameState* s)
{
    s->model_count = 0;
    s->shader_count = 0;

    rd_init(&s->rd, GAME_WIDTH, GAME_HEIGHT, "cool stuff");

    // better method to use a counter instead of manually keeping track of numbers
    s->shader_indices[s->shader_count] = rd_add_shader(&s->rd, "shaders/model_default.vert", "shaders/model_default.frag");
    s->shader_count++;

    // cam setup
    vec3 start_pos = {0.0f, 0.0f, 3.0f};
    float aspect_ratio = (float)(s->rd.width) / (float)(s->rd.height);
    s->cam = camera_init(start_pos, -90.0f, 0.0f);
    camera_update_proj(&s->cam, 90.0f, aspect_ratio, 0.01f, 100.0f);

    model_load(&s->models[0], "res/backpack/backpack.obj");
    s->model_count++;

    s->sphere = uv_sphere_gen((vec3){5.0f, 3.0f, 0.0f}, 2.0f, 15, "res/earth/e.png");
    s->skybox = skybox_init("res/meadow/m.png");
}

void game_update(GameState* s)
{
    Shader* model_shader = &s->rd.shaders[s->shader_indices[0]]; 
    camera_update(&s->cam, &s->rd);

    shader_use(model_shader);

    mat4 vp;
    mat4_mul(&vp, s->cam.proj, s->cam.view);

    shader_uniform_mat4(model_shader, "vp", &vp);

    mat4 model = mat4_identity();
    mat4_rotate_y(&model, rd_get_time()); // GET TICKS
    mat4_scale_scalar(&model, 1.0f);
    shader_uniform_mat4(model_shader, "model", &model);

    for(uint32_t i = 0; i < s->model_count; i++)
        model_draw(&s->models[i], model_shader);

    sphere_draw(&s->sphere, &vp);

    // must be drawn last after everything else has filled the depth buffer
    skybox_draw(&s->skybox, &s->cam);
}

void game_end(GameState* s)
{
    for(uint32_t i = 0; i < s->model_count; i++)
    {
        model_free(&s->models[i]);
    }

    skybox_free(&s->skybox);
    sphere_free(&s->sphere);
    rd_free(&s->rd);
}