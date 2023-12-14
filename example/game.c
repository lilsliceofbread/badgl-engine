#include "game.h"

#include <stdio.h>
#include <stdbool.h>
#include "glmath.h"
#include "renderer.h"
#include "camera.h"
#include "util.h"
#include "model.h"
#include "platform.h"
#include "sphere.h"
#include "quad.h"

#define GAME_WIDTH 1280
#define GAME_HEIGHT 720

static bool is_vsync_on = true;

void game_init(GameState* s)
{
    s->scene_count = 0;
    s->current_scene = 0;

    rd_init(&s->rd, GAME_WIDTH, GAME_HEIGHT, "cool stuff");

    {
        Quad loading_screen = quad_init((vec2){-1.0f, -1.0f}, (vec2){2.0f, 2.0f}, "res/loading.png");
        rd_update_viewport(&s->rd); // glfw framebuffer size may not have updated yet so update renderer width/height
        int size = (s->rd.width >= s->rd.height) ? s->rd.height >> 1: s->rd.width >> 1;
        rd_set_viewport((s->rd.width >> 1) - (size >> 1), (s->rd.height >> 1) - (size >> 1), size, size); // place loading in middle of screen
        quad_draw(&loading_screen, &s->rd);
        rd_swap_buffers(&s->rd);
        quad_free(&loading_screen);
    }

    vec3 start_pos = {0.0f, 0.0f, 3.0f};
    vec2 start_euler = {0.0f, -90.0f};
    scene_init(&s->scenes[s->scene_count++], start_pos, start_euler, &s->rd, "res/kurt/space.png");
    scene_init(&s->scenes[s->scene_count++], start_pos, start_euler, &s->rd, "res/box/box.png");

    {
        // we create these shaders ourselves and then pass the index to the structures so multiple can use the same shader
        uint32_t model_shader = rd_add_shader(&s->rd, "shaders/model_default.vert", "shaders/model_default.frag");
        uint32_t sphere_shader = rd_add_shader(&s->rd, "shaders/sphere.vert", "shaders/sphere.frag");

        Model tmp = uv_sphere_gen(2.0f, 15, "res/earth/e.png", sphere_shader);
        scene_add_model(&s->scenes[0], &tmp);
        tmp = model_load("res/backpack/backpack.obj", model_shader);
        scene_add_model(&s->scenes[1], &tmp);
    }
    
    Transform backpack = {
        .pos = (vec3){5.0f, 0.0f, 2.0f},
        .euler = (vec3){0.0f, 0.0f, 0.0f},
        .scale = (vec3){1.0f, 1.0f, 1.0f}
    };
    model_update_transform(&s->scenes[1].models[0], &backpack);

    rd_update_viewport(&s->rd); // reset back after loading screen (begin/end load func?)
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

        if(igButton("Next Scene",(struct ImVec2){0,0}))
        {
            s->current_scene = (s->current_scene + 1) % s->scene_count;
        }
    igEnd();

    scene_update(&s->scenes[s->current_scene], &s->rd);

    // bit scuffed
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