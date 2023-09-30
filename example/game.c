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
    rd_init(&s->rd, GAME_WIDTH, GAME_HEIGHT, "cool stuff");
    s->shader_index = rd_add_shader(&s->rd, "shaders/default.vert", "shaders/default.frag");

    // cam starting position and angle
    vec3 start_pos = {0.0f, 0.0f, 3.0f};
    s->cam = camera_init(start_pos, -90.0f, 0.0f);
    vec3_copy((vec3){0.0f, 0.0f, 3.0f}, &s->cam.pos);

    float aspect_ratio = (float)(s->rd.width) / (float)(s->rd.height);
    camera_calc_view_vecs(&s->cam);
    camera_calc_proj(&s->cam, 90.0f, aspect_ratio, 0.01f, 100.0f);

    rd_set_cam_bool(&s->rd, &s->cam.wait_update);

    //model_load(&s->models[0], "res/backpack/backpack.obj");
    s->sphere = gen_uv_sphere((vec3){0.0f, 3.0f, 0.0f}, 2.0f, 9, 17, "res/test/beans2.png");
}

void game_update(GameState* s, float curr_time, float delta_time)
{
    Shader* shader = rd_get_shader(&s->rd, s->shader_index);
    camera_update(&s->cam, &s->rd, delta_time);

    // multiply view and proj matrices, and send to uniform on shader
    shader_use(shader);
    mat4 vp = mat4_mul(s->cam.proj, s->cam.view);
    shader_uniform_mat4(shader, "vp", vp);

    // could do in model_draw !!!
    //
    //
    mat4 model = mat4_identity();
    model = mat4_rotate_y(model, curr_time);
    //model = mat4_scale_scalar(model, 1.0f);
    shader_uniform_mat4(shader, "model", model);

    sphere_draw(&s->sphere, shader);
    // will do in loop with model count
    //model_draw(&s->models[0], shader);
}

void game_end(GameState* s)
{
    //model_free(&s->models[0]);
    sphere_free(&s->sphere);
    rd_free(&s->rd);
}