#include "game.h"

#include <stdio.h>
#include <stdbool.h>
#include "glmath.h"
#include "renderer.h"
#include "camera.h"
#include "util.h"
#include "model.h"
#include "platform.h"
#include "shapes.h"
#include "quad.h"

#define GAME_WIDTH 1280
#define GAME_HEIGHT 720

void loading_begin(Renderer* rd);
void loading_end(Renderer* rd);

void game_init(GameState* s)
{
    s->scene_count = 0;
    s->current_scene = 0;
    s->is_vsync_on = true;

    rd_init(&s->rd, GAME_WIDTH, GAME_HEIGHT, "cool stuff");

    loading_begin(&s->rd);

    vec3 start_pos = {0.0f, 0.0f, 3.0f};
    vec2 start_euler = {0.0f, -90.0f}; // pitch then yaw
    scene_init(&s->scenes[s->scene_count++], start_pos, start_euler, &s->rd, "res/kurt/space.png");
    scene_init(&s->scenes[s->scene_count++], start_pos, start_euler, &s->rd, "res/box/box.png");

    {
        // we create these shaders ourselves and then pass the index to the structures so multiple can use the same shader
        uint32_t model_tex_shader = rd_add_shader(&s->rd, "shaders/model_tex.vert", "shaders/model_tex.frag");
        uint32_t default_shader = rd_add_shader(&s->rd, "shaders/default.vert", "shaders/default.frag");
        uint32_t sphere_shader = rd_add_shader(&s->rd, "shaders/sphere.vert", "shaders/sphere.frag");

        Model tmp = uv_sphere_gen(2.0f, 15, "res/earth/e.png", (vec3){1.0f, 1.0f, 1.0f}, sphere_shader);
        scene_add_model(&s->scenes[0], &tmp);
        tmp = model_load("res/backpack/backpack.obj", model_tex_shader);
        scene_add_model(&s->scenes[1], &tmp);
        tmp = rectangular_prism_gen(1.5f, 2.0f, 3.0f, NULL, (vec3){0.7f, 0.2f, 0.8f}, default_shader);
        scene_add_model(&s->scenes[1], &tmp);
    }
    
    Transform backpack = {
        .pos = (vec3){5.0f, 0.0f, 2.0f},
        .euler = (vec3){0.0f, 0.0f, 0.0f},
        .scale = (vec3){1.0f, 1.0f, 1.0f}
    };
    model_update_transform(&s->scenes[1].models[0], &backpack);

    loading_end(&s->rd);
}

void game_update(GameState* s)
{
    igSetNextWindowPos((ImVec2){0,0}, ImGuiCond_FirstUseEver, (ImVec2){0,0});
    igBegin("Settings", NULL, 0);
        igText("FPS: %f", 1.0f / s->rd.delta_time);

        if(igButton("Toggle V-Sync",(struct ImVec2){0,0}))
        {
            platform_toggle_vsync(!s->is_vsync_on);
            s->is_vsync_on = !s->is_vsync_on; 
        }

        if(igButton("Next Scene",(struct ImVec2){0,0}))
        {
            s->current_scene = (s->current_scene + 1) % s->scene_count;
        }
    igEnd();

    scene_update(&s->scenes[s->current_scene], &s->rd);

    if(s->current_scene == 0)
    {
        Transform sphere = {
            .pos = (vec3){0.0f, 0.0f, 0.0f},
            .euler = (vec3){0.0f, rd_get_time(), 0.0f},
            .scale = (vec3){1.0f, 1.0f, 1.0f}
        };
        model_update_transform(&s->scenes[0].models[0], &sphere);
    }

    scene_draw(&s->scenes[s->current_scene], &s->rd);
}

void game_end(GameState* s)
{
    for(uint32_t i = 0; i < s->scene_count; i++)
    {
        scene_free(&s->scenes[i]);
    }
    rd_free(&s->rd);
}

void loading_begin(Renderer* rd)
{
    Quad loading_screen = quad_init((vec2){-1.0f, -1.0f}, (vec2){2.0f, 2.0f}, "res/loading.png");

    rd_update_viewport(rd); // glfw framebuffer size may not have updated yet so update renderer width/height

    int size = (rd->width >= rd->height) ? rd->height >> 1: rd->width >> 1;
    rd_set_viewport((rd->width >> 1) - (size >> 1), (rd->height >> 1) - (size >> 1), size, size); // place loading in middle of screen

    quad_draw(&loading_screen, rd);
    rd_swap_buffers(rd);
    quad_free(&loading_screen);
}

void loading_end(Renderer* rd)
{
    rd_update_viewport(rd); // reset back after loading screen
}