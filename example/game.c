#include "game.h"

#include <cglm/cglm.h>
#include <stdio.h>
#include <stdbool.h>
#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "util.h"
#include "model.h"
#include "vao.h"
#include "bo.h"

#define GAME_WIDTH 1280
#define GAME_HEIGHT 720

void _mouse_callback(GLFWwindow* window, double cursor_x, double cursor_y);
void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

static bool cursor_disabled = true;

void game_init(GameState* state)
{
    Window* win = &(state->rd.win);
    
    rd_init(&(state->rd), GAME_WIDTH, GAME_HEIGHT, "Game");
    window_set_callback_ptr(win, &(state->rd));
    window_key_callback(win, _key_callback);
    window_mouse_callback(win, _mouse_callback);

    // cam starting position and angle
    Camera* cam = &(state->rd.cam);
    glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, cam->pos);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam->dir); // looking down negative z

    model_load(&(state->models[0]), "res/backpack.obj");
}

void game_update(GameState* state, float curr_time, float delta_time)
{
    rd_clear_frame();

    // will be player, cam should be in state not renderer
    camera_process_inputs(&(state->rd.cam), &(state->rd.win), delta_time);

    // multiply view and proj matrices, and send to uniform on shader
    rd_send_vp_matrix(&(state->rd));

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
    window_terminate();
}

void _mouse_callback(GLFWwindow* window, double cursor_x, double cursor_y)
{
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(window);
    Camera* cam = &(rd->cam);
    camera_mouse_update(cam, cursor_x, cursor_y);
}

void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(window);

    if(key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1); // true
    }
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if(cursor_disabled) // toggle between cursor locked vs usable
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            rd->cam.wait_update = true; // to stop sharp turn when cursor jumps
            cursor_disabled = false;
        }
        else
        {
            glfwFocusWindow(window);
            glfwSetCursorPos(window, rd->win.width/2, rd->win.height/2); // move cursor to middle of screen
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            rd->cam.wait_update = true; // to stop sharp turn when cursor jumps
            cursor_disabled = true;
        }
    }
}
