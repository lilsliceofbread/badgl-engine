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

void game_init(GameState* state)
{
    rd_init(&(state->rd), GAME_WIDTH, GAME_HEIGHT, "Game");
    state->shader_index = rd_add_shader(&(state->rd), "shaders/default.vert", "shaders/default.frag");

    // cam starting position and angle
    state->cam = camera_init();
    vec3_copy((vec3){0.0f, 0.0f, 3.0f}, &(state->cam.pos));
    vec3_copy((vec3){0.0f, 0.0f, -1.0f}, &(state->cam.dir)); // looking down negative z

    float aspect_ratio = (float)(state->rd.width) / (float)(state->rd.height);
    camera_calc_view_vecs(&(state->cam));
    camera_calc_proj(&(state->cam), 90.0f, aspect_ratio, 0.1f, 100.0f);

    rd_set_cam_bool(&(state->rd), &(state->cam.wait_update));

    model_load(&(state->models[0]), "res/backpack/backpack.obj");
}

void game_update(GameState* state, float curr_time, float delta_time)
{
    Shader* shader = rd_get_shader(&(state->rd), state->shader_index);
    camera_update(&(state->cam), &(state->rd), delta_time);

    // multiply view and proj matrices, and send to uniform on shader
    shader_use(shader);
    //mat4_test vp = mat4_test_mul(state->cam.proj, state->cam.view);
    shader_uniform_mat4(shader, "view", state->cam.view);
    shader_uniform_mat4(shader, "proj", state->cam.proj);

    // could do in model_draw
    mat4 model = mat4_identity();
    model = mat4_scale(model, (vec3){0.5f, 0.5f, 0.5f});
    model = mat4_rotate_y(model, curr_time);
    shader_uniform_mat4(shader, "model", model);

    // will do in loop with model count
    model_draw(&(state->models[0]), shader);
}

void game_end(GameState* state)
{
    model_free(&(state->models[0]));
    rd_free(&(state->rd));
}