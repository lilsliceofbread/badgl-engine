#include "game.h"

#include <stdio.h>
#include <stdbool.h>
#include "glmath.h"
#include "renderer.h"
#include "camera.h"
#include "util.h"
#include "model.h"
#include "platform.h"

#define GAME_WIDTH 1280
#define GAME_HEIGHT 720

static bool is_vsync_on = true;

void game_init(GameState* s)
{
    s->model_count = 0;

    rd_init(&s->rd, GAME_WIDTH, GAME_HEIGHT, "cool stuff");

    vec3 start_pos = {0.0f, 0.0f, 3.0f};
    const float aspect_ratio = (float)(s->rd.width) / (float)(s->rd.height); // not casting these to float causes the aspect ratio to be rounded
    s->cam = camera_init(start_pos, -90.0f, 0.0f);
    camera_update_proj(&s->cam, 90.0f, aspect_ratio, 0.01f, 100.0f);

    // we create these shader's ourselves and then pass the index to the structures so multiple can use the same shader
    uint32_t model_shader = rd_add_shader(&s->rd, "shaders/model_default.vert", "shaders/model_default.frag");
    uint32_t sphere_shader = rd_add_shader(&s->rd, "shaders/sphere.vert", "shaders/sphere.frag");

    model_load(&s->models[s->model_count], "res/backpack/backpack.obj", model_shader);
    s->model_count++;
    uv_sphere_gen(&s->models[s->model_count], 2.0f, 15, "res/earth/e.png", sphere_shader);
    s->model_count++;
    
    Transform backpack;
    transform_reset(&backpack);
    vec3_copy((vec3){5.0f, 0.0f, 2.0f}, &backpack.pos);
    model_update_transform(&s->models[0], &backpack);

    s->skybox = skybox_init("res/box/box.png");
}

void game_update(GameState* s)
{
    igSetNextWindowPos((ImVec2){0,0}, ImGuiCond_FirstUseEver, (ImVec2){0,0});
    igBegin("Settings", NULL, 0);
        igText("FPS: %f", 1.0f / s->rd.delta_time);
        if(igButton("Toggle V-Sync",(struct ImVec2){0,0}))
        {
            platform_toggle_vsync(!is_vsync_on);
            is_vsync_on = !is_vsync_on; 
        }
    igEnd();

    camera_update(&s->cam, &s->rd);

    mat4 vp;
    mat4_mul(&vp, s->cam.proj, s->cam.view);

    Transform sphere;
    transform_reset(&sphere);
    vec3_copy((vec3){0.0f, rd_get_time(), 0.0f}, &sphere.euler);
    model_update_transform(&s->models[1], &sphere);

    for(uint32_t i = 0; i < s->model_count; i++)
    {
        model_draw(&s->models[i], &s->rd, &vp);
    }

    skybox_draw(&s->skybox, &s->cam); // must be drawn last after everything else has filled the depth buffer
}

void game_end(GameState* s)
{
    for(uint32_t i = 0; i < s->model_count; i++)
    {
        model_free(&s->models[i]);
    }

    skybox_free(&s->skybox);
    rd_free(&s->rd);
}