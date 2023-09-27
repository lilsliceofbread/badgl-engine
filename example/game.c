#include "game.h"

#include <cglm/cglm.h>
#include <stdio.h>
#include <stdbool.h>
#include "renderer.h"
#include "camera.h"
#include "util.h"
#include "model.h"

#define GAME_WIDTH 1280
#define GAME_HEIGHT 720

void _mouse_callback(GLFWwindow* window, double cursor_x, double cursor_y);
void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


void game_init(GameState* state)
{
    rd_init(&(state->rd), GAME_WIDTH, GAME_HEIGHT, "Game", "shaders/default.vert", "shaders/default.frag");

    // cam starting position and angle
    state->cam = camera_init(); // should be user defined
    glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, state->cam.pos);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, state->cam.dir); // looking down negative z

    float aspect_ratio = (float)(state->rd.width) / (float)(state->rd.height);
    camera_calc_proj(&(state->cam), 90.0f, aspect_ratio, 0.1f, 100.0f); // remove to user

    model_load(&(state->models[0]), "res/backpack.obj");

    // temp
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void game_update(GameState* state, float curr_time, float delta_time)
{
    camera_update(&(state->cam), &(state->rd), delta_time);

    // multiply view and proj matrices, and send to uniform on shader
    // should be user defined / separate from renderer
    shader_use(&(state->rd.shader));
    mat4 vp;
    glm_mat4_mul(state->cam.proj, state->cam.view, vp);
    shader_uniform_mat4(&(state->rd.shader), "vp", vp);

    mat4 model;
    glm_mat4_identity(model);
    glm_rotate(model, curr_time, (vec3){0.0f, 1.0f, 0.0f});
    glm_scale(model, (vec3){0.5f, 0.5f, 0.5f});
    shader_uniform_mat4(&(state->rd.shader), "model", model);
    
    // will do in loop with model count
    model_draw(&(state->models[0]), &(state->rd.shader));
}

void game_end(GameState* state)
{
    model_free(&(state->models[0]));
    rd_free(&(state->rd));
}