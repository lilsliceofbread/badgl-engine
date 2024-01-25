#include "game.h"

#include <stdio.h>
#include <stdbool.h>
#include "glmath.h"
#include "scene.h"
#include "renderer.h"
#include "camera.h"
#include "util.h"
#include "model.h"
#include "platform.h"
#include "shapes.h"
#include "quad.h"

#define MAX_SCENES 5

static struct
{
    Renderer rd;
    Scene scenes[MAX_SCENES];
    u32 current_scene;
    u32 scene_count;
    bool is_vsync_on;
} s; // game state

void game_add_models(void);
void game_add_lights(void);
void loading_begin(Renderer* rd);
void loading_end(Renderer* rd);
void sphere_scene_update(Scene* scene);

void game_init(void)
{
    s.scene_count = 0;
    s.current_scene = 0;
    s.is_vsync_on = true;

    rd_init(&s.rd, 1280, 720, "badgl demo", RD_USE_SKYBOX | RD_USE_UI | RD_USE_LIGHTING);

    loading_begin(&s.rd);

    vec3 start_pos = VEC3(0.0f, 1.0f, 3.0f);
    vec2 start_euler = VEC2(0.0f, -90.0f); // pitch then yaw

    scene_create(&s.scenes[s.scene_count++], &s.rd, start_pos, start_euler);
    scene_set_skybox(&s.scenes[0], &s.rd, "res/kurt/space.png");
    scene_set_update_callback(&s.scenes[0], (SceneUpdateFunc)sphere_scene_update);

    scene_create(&s.scenes[s.scene_count++], &s.rd, start_pos, start_euler);
    scene_set_skybox(&s.scenes[1], &s.rd, "res/hills.png");

    game_add_models();
    game_add_lights();

    scene_update_lights(&s.scenes[s.current_scene], &s.rd);

    loading_end(&s.rd);
}

void game_run(void)
{
    while(!rd_win_should_close(&s.rd))
    {
        rd_begin_frame(&s.rd);

        igBegin("settings", NULL, 0);
            igText("fps: %f", 1.0f / s.rd.delta_time);

            if(igButton("toggle v-sync",(struct ImVec2){0,0}))
            {
                platform_toggle_vsync(!s.is_vsync_on);
                s.is_vsync_on = !s.is_vsync_on; 
            }

            if(igButton("next scene",(struct ImVec2){0,0}))
            {
                s.current_scene = (s.current_scene + 1) % s.scene_count;
                scene_switch(&s.scenes[s.current_scene], &s.rd);
            }
        igEnd();

        scene_update(&s.scenes[s.current_scene], &s.rd);

        scene_draw(&s.scenes[s.current_scene], &s.rd);

        rd_end_frame(&s.rd);
    }
}

void game_end(void)
{
    for(u32 i = 0; i < s.scene_count; i++)
    {
        scene_free(&s.scenes[i]);
    }
    rd_free(&s.rd);
}

void loading_begin(Renderer* rd)
{
    Quad loading_screen = quad_create((vec2){-1.0f, -1.0f}, (vec2){2.0f, 2.0f}, "res/loading.png");

    rd_update_viewport(rd); // glfw framebuffer size may not have updated yet so update renderer width/height

    int size = (rd->width >= rd->height) ? rd->height >> 1 : rd->width >> 1;
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
    const float rotate_speed = 50.0f;
    float time = (float)platform_get_time();

    Transform sphere = scene->models[0].transform;
    sphere.euler = VEC3(0.0f, rotate_speed * time, 0.0f);
    model_update_transform(&scene->models[0], &sphere);
}

void game_add_models(void)
{
    // create shaders ourselves then pass index to structures so multiple use same shader
    u32 model_shader = rd_add_shader(&s.rd, "shaders/model.vert", "shaders/model.frag");
    u32 sphere_shader = rd_add_shader(&s.rd, "shaders/sphere.vert", "shaders/sphere.frag");

    Material material = {0}; 
    Model model_tmp = {0};
    Transform transform;
    transform_reset(&transform);

    /* scene 0 */

    material_texture_diffuse(&material, true, "res/earth/e.png",
                                VEC3(1.0f, 1.0f, 1.0f), 32.0f);
    shapes_uv_sphere(&model_tmp, 15, &material, sphere_shader);
    scene_add_model(&s.scenes[0], &model_tmp);
    transform.scale = VEC3(2.0f, 2.0f, 2.0f);
    model_update_transform(&s.scenes[0].models[0], &transform); // TODO: use some identifier for models? strings?

    material_textureless(&material, true,
                            VEC3(0.8f, 0.1f, 0.2f),
                            VEC3(0.8f, 0.1f, 0.2f),
                            VEC3(1.0f, 1.0f, 1.0f), 32.0f);
    shapes_uv_sphere(&model_tmp, 15, &material, sphere_shader);
    scene_add_model(&s.scenes[0], &model_tmp);
    transform.pos = VEC3(5.0f, 0.0f, -2.0f);
    transform.scale = VEC3(2.0f, 2.0f, 2.0f);
    model_update_transform(&s.scenes[0].models[1], &transform);

    transform.scale = VEC3(1.0f, 1.0f, 1.0f);

    /* scene 1 */

    material_textureless(&material, false,
                            VEC3(0.3f, 0.2f, 0.8f),
                            VEC3(0.3f, 0.2f, 0.8f),
                            VEC3(1.0f, 1.0f, 1.0f), 32.0f);
    shapes_rectangular_plane(&model_tmp, 100.0f, 100.0f, 10, &material, model_shader);
    scene_add_model(&s.scenes[1], &model_tmp);

    model_load(&model_tmp, "res/backpack/backpack.obj", &material, model_shader);
    scene_add_model(&s.scenes[1], &model_tmp);
    transform.pos = VEC3(3.0f, 3.0f, 0.0f);
    model_update_transform(&s.scenes[1].models[1], &transform);

    material_textureless(&material, false,
                            VEC3(0.8f, 0.2f, 0.3f),
                            VEC3(0.8f, 0.2f, 0.3f),
                            VEC3(1.0f, 1.0f, 1.0f), 32.0f);
    shapes_rectangular_prism(&model_tmp, 1.5f, 2.0f, 3.0f, &material, model_shader);
    scene_add_model(&s.scenes[1], &model_tmp);
    transform.pos = VEC3(-2.0f, 2.0f, 0.0f);
    model_update_transform(&s.scenes[1].models[2], &transform);

    material_textureless(&material, true,
                            VEC3(0.8f, 0.1f, 0.2f),
                            VEC3(0.8f, 0.1f, 0.2f),
                            VEC3(1.0f, 1.0f, 1.0f), 32.0f);
    shapes_uv_sphere(&model_tmp, 15, &material, sphere_shader);
    scene_add_model(&s.scenes[1], &model_tmp);
    transform.pos = VEC3(-8.0f, 5.0f, 3.0f);
    transform.scale = VEC3(4.0f, 4.0f, 4.0f);
    model_update_transform(&s.scenes[1].models[3], &transform);
}

void game_add_lights(void)
{
    Model model_tmp;
    Light light;
    DirLight dir_light;
    Transform transform;
    transform_reset(&transform);
    transform.scale = VEC3(0.5f, 0.5f, 0.5f);

    /* scene 0 */

    shapes_uv_sphere(&model_tmp, 10, NULL, 0); // don't need to worry about setting shader, scene will do that for us
    model_update_transform(&model_tmp, &transform);
    light_create(&light, VEC3(0.0f, 5.0f, 5.0f), 
                         VEC3(0.2f, 0.2f, 0.2f),
                         VEC3(0.6f, 0.6f, 0.6f),
                         VEC3(0.8f, 0.8f, 0.8f),
                         VEC3(0.003f, 0.007f, 1.0f));
    scene_add_light(&s.scenes[0], &s.rd, &light, &model_tmp);

    /* scene 1 */

    shapes_uv_sphere(&model_tmp, 10, NULL, 0);
    model_update_transform(&model_tmp, &transform);
    light_create(&light, VEC3(10.0f, 5.0f, 5.0f),
                         VEC3(0.0f, 0.0f, 0.2f),
                         VEC3(0.0f, 0.0f, 0.6f),
                         VEC3(0.0f, 0.0f, 0.8f),
                         VEC3(0.007f, 0.014f, 1.0f));
    scene_add_light(&s.scenes[1], &s.rd, &light, &model_tmp);

    shapes_uv_sphere(&model_tmp, 10, NULL, 0);
    model_update_transform(&model_tmp, &transform);
    light_create(&light, VEC3(-5.0f, 5.0f, -5.0f),
                         VEC3(0.2f, 0.0f, 0.0f),
                         VEC3(0.5f, 0.0f, 0.0f),
                         VEC3(0.8f, 0.0f, 0.0f),
                         VEC3(0.007f, 0.014f, 1.0f));
    scene_add_light(&s.scenes[1], &s.rd, &light, &model_tmp);

    dir_light_create(&dir_light, VEC3(-4.0f, -12.0f, 10.0f), // roughly the position of the sun on the skybox
                                 VEC3(0.2f, 0.2f, 0.2f),
                                 VEC3(0.5f, 0.5f, 0.5f),
                                 VEC3(0.8f, 0.8f, 0.8f));
    scene_set_dir_light(&s.scenes[1], &dir_light);
}