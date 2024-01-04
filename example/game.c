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
void sphere_scene_update(Scene* scene);

void game_init(GameState* s)
{
    s->scene_count = 0;
    s->current_scene = 0;
    s->is_vsync_on = true;

    rd_init(&s->rd, GAME_WIDTH, GAME_HEIGHT, "cool stuff");

    loading_begin(&s->rd);

    vec3 start_pos = {0.0f, 1.0f, 3.0f};
    vec2 start_euler = {0.0f, -90.0f}; // pitch then yaw
    scene_init(&s->scenes[s->scene_count++], start_pos, start_euler,
               &s->rd, "res/kurt/space.png", (SceneUpdateFunc)sphere_scene_update);
    scene_init(&s->scenes[s->scene_count++], start_pos, start_euler,
               &s->rd, "res/box/box.png", NULL);

    {
        // create shaders ourselves then pass index to structures so multiple use same shader
        uint32_t model_shader = rd_add_shader(&s->rd, "shaders/model.vert", "shaders/model.frag");
        //uint32_t default_shader = rd_add_shader(&s->rd, "shaders/default.vert", "shaders/default.frag");
        uint32_t sphere_shader = rd_add_shader(&s->rd, "shaders/sphere.vert", "shaders/sphere.frag");

        Material mat_tmp; 
        material_texture_diffuse(&mat_tmp, true, "res/earth/e.png",
                                 (vec3){1.0f, 1.0f, 1.0f}, 32.0f);
        Model model_tmp = uv_sphere_gen(2.0f, 15, &mat_tmp, sphere_shader);
        scene_add_model(&s->scenes[0], &model_tmp);

        material_textureless(&mat_tmp, true,
                             (vec3){0.8f, 0.1f, 0.2f},
                             (vec3){0.8f, 0.1f, 0.2f},
                             (vec3){1.0f, 1.0f, 1.0f}, 32.0f);
        model_tmp = uv_sphere_gen(2.0f, 15, &mat_tmp, sphere_shader);
        scene_add_model(&s->scenes[0], &model_tmp);

        material_textureless(&mat_tmp, false,
                             (vec3){0.3f, 0.2f, 0.8f},
                             (vec3){0.3f, 0.2f, 0.8f},
                             (vec3){1.0f, 1.0f, 1.0f}, 32.0f);
        model_tmp = rectangular_plane_gen(100.0f, 100.0f, 10, &mat_tmp, model_shader);
        scene_add_model(&s->scenes[1], &model_tmp);

        model_tmp = model_load("res/backpack/backpack.obj", model_shader, &mat_tmp);
        scene_add_model(&s->scenes[1], &model_tmp);

        material_textureless(&mat_tmp, false,
                             (vec3){0.8f, 0.2f, 0.3f},
                             (vec3){0.8f, 0.2f, 0.3f},
                             (vec3){1.0f, 1.0f, 1.0f}, 32.0f);
        model_tmp = rectangular_prism_gen(1.5f, 2.0f, 3.0f, &mat_tmp, model_shader);
        scene_add_model(&s->scenes[1], &model_tmp);
    }
    
    {
        Transform tr_tmp = {
            .pos   = (vec3){5.0f, 0.0f, -2.0f},
            .euler = (vec3){0.0f, 0.0f, 0.0f},
            .scale = (vec3){1.0f, 1.0f, 1.0f}
        };
        // TODO: use some identifier for models e.g. strings?
        model_update_transform(&s->scenes[0].models[1], &tr_tmp);

        tr_tmp.pos = (vec3){-2.0f, 2.0f, 0.0f},
        model_update_transform(&s->scenes[1].models[2], &tr_tmp);

        tr_tmp.pos = (vec3){3.0f, 3.0f, 0.0f},
        model_update_transform(&s->scenes[1].models[1], &tr_tmp);
    }

    loading_end(&s->rd);
}

void game_update(GameState* s)
{
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

void sphere_scene_update(Scene* scene)
{
    Transform sphere = {
        .pos = (vec3){0.0f, 0.0f, 0.0f},
        .euler = (vec3){0.0f, rd_get_time(), 0.0f},
        .scale = (vec3){1.0f, 1.0f, 1.0f}
    };
    model_update_transform(&scene->models[0], &sphere);
}