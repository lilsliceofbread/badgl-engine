#include "game.h"

#include <stdio.h>
#include <stdbool.h>
#include "badgl.h"

#define MAX_SCENES 5

static struct
{
    Renderer rd;
    Scene scenes[MAX_SCENES];
    u32 scene_count;

    u32 current_scene;
    i32 light_index;
    bool is_vsync_on;
} s; // game state

void game_add_models(Arena* arena);
void game_add_lights(Arena* arena);
void loading_begin(void);
void loading_end(void);
void sphere_scene_update(Scene* scene);

void game_init()
{
    s.scene_count = 0;
    s.current_scene = 0;
    s.is_vsync_on = true;

    Arena arena;
    arena_create(&arena); // create arena for loading data
                          
    RendererFlags flags = 0; // use defaults - can disable lighting, skybox e.g. BGL_RD_SKYBOX_OFF | BGL_RD_UI_OFF | BGL_RD_LIGHTING_OFF
    rd_init(&s.rd, 1280, 720, "badgl demo", flags, "4.6"); // my pc has max opengl 4.2 so no debug output for me :(

    loading_begin(); // a simple quad is drawn to the screen before loading

    vec3 start_pos = VEC3(0.0f, 1.0f, 3.0f); // x, y, z
    vec2 start_euler = VEC2(0.0f, -90.0f); // pitch then yaw

    scene_create(&s.scenes[s.scene_count++], &s.rd, start_pos, start_euler);
    scene_set_skybox(&s.scenes[0], &arena, &s.rd, "res/kurt/space.png");
    scene_set_update_callback(&s.scenes[0], (SceneUpdateFunc)sphere_scene_update);

    scene_create(&s.scenes[s.scene_count++], &s.rd, start_pos, start_euler);
    scene_set_skybox(&s.scenes[1], &arena, &s.rd, "res/hills.png");

    game_add_models(&arena);
    game_add_lights(&arena);

    arena_free(&arena); // free arena - done with loading

    scene_switch(&s.scenes[s.current_scene], &s.rd); // call this when switching to a new scene

    loading_end();
}

void game_run(void)
{
    while(!rd_win_should_close(&s.rd))
    {
        rd_begin_frame(&s.rd);

        igBegin("settings", NULL, 0);
            igText("fps: %f", 1.0f / s.rd.delta_time);

            if(igButton("toggle v-sync", (ImVec2){0, 0}))
            {
                platform_toggle_vsync(!s.is_vsync_on);
                s.is_vsync_on = !s.is_vsync_on; 
            }

            if(igButton("next scene", (ImVec2){0, 0}))
            {
                s.current_scene = (s.current_scene + 1) % s.scene_count;
                scene_switch(&s.scenes[s.current_scene], &s.rd);
            }

            {
                igText("light editor");

                Scene* scene = &s.scenes[s.current_scene];

                igInputInt("current light", &s.light_index, 1, 1, 0);
                s.light_index = CLAMP(s.light_index, 0, scene->light_count - 1);

                Light* light = &scene->lights[s.light_index];
                DirLight* dir_light = &scene->dir_light;

                igText("position:");
                igInputFloat("x", (f32*)&light->pos.x, 0.5f, 0.5f, "%.1f", 0);
                igInputFloat("y", (f32*)&light->pos.y, 0.5f, 0.5f, "%.1f", 0);
                igInputFloat("z", (f32*)&light->pos.z, 0.5f, 0.5f, "%.1f", 0);
                igColorEdit3("ambient", (f32*)&light->ambient, 0);
                igColorEdit3("diffuse", (f32*)&light->diffuse, 0);
                igColorEdit3("specular", (f32*)&light->specular, 0);
                igSliderFloat3("attenuation", (f32*)&light->attenuation, 0.0f, 1.0f, "%.3f", 0);

                igDummy((ImVec2){1, 1}); // spacing
                igText("directional light");

                igSliderFloat3("direction", (f32*)&dir_light->dir, -1.0f, 1.0f, "%.2f", 0);
                igColorEdit3("ambient##1", (f32*)&dir_light->ambient, 0);
                igColorEdit3("diffuse##1", (f32*)&dir_light->diffuse, 0);
                igColorEdit3("specular##1", (f32*)&dir_light->specular, 0);

                scene_update_lights(scene, &s.rd); // this updates light data and light graphics as well, because we are rendering this scene
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

void loading_begin(void)
{
    Quad loading_screen = quad_create(VEC2(-1.0f, -1.0f), VEC2(2.0f, 2.0f), "res/loading.png");

    rd_update_viewport(&s.rd); // glfw framebuffer size may not have updated yet so update renderer width/height

    i32 size = (s.rd.width >= s.rd.height) ? s.rd.height / 2 : s.rd.width / 2;
    rd_set_viewport((s.rd.width / 2) - (size / 2), (s.rd.height / 2) - (size / 2), size, size); // place loading in middle of screen

    quad_draw(&loading_screen, &s.rd);
    rd_swap_buffers(&s.rd);
    quad_free(&loading_screen);
}

void loading_end(void)
{
    rd_update_viewport(&s.rd); // reset back after loading screen
}

void sphere_scene_update(Scene* scene)
{
    const f32 rotate_speed = 50.0f;
    const f32 time = (f32)platform_get_time();

    Transform sphere = scene->models[0].transform;
    sphere.euler = VEC3(0.0f, rotate_speed * time, 0.0f);
    model_update_transform(&scene->models[0], &sphere); // TODO: use some identifier for models in scene? strings?
}

void game_add_models(Arena* arena)
{
    /* create shaders for our models */

    u32 model_shader, sphere_shader;
    const char* shader_filepaths[] = {"shaders/model.vert", "shaders/model.frag", "shaders/sphere.vert", "shaders/sphere.frag"};
    if(!rd_add_shader(&s.rd, arena, &shader_filepaths[0], 2, &model_shader)
    || !rd_add_shader(&s.rd, arena, &shader_filepaths[2], 2, &sphere_shader))
    {
        BGL_LOG_ERROR("cannot continue without shaders, ending game");
        game_end();
        exit(-1);
    }

    Material materials[5] = {0}; 
    Model models[6] = {0};
    Transform transforms[6] = {0};

    /* creating materials */

    material_create(&materials[0], true,
                    VEC3(0.0f, 0.0f, 0.0f),
                    VEC3(0.0f, 0.0f, 0.0f),
                    VEC3(0.0f, 0.0f, 0.0f), 32.0f);
    material_add_texture(&materials[0], TEXTURE_DIFFUSE, "res/earth_day_diffuse.png");
    material_add_texture(&materials[0], TEXTURE_SPECULAR, "res/earth_specular.png");

    material_create(&materials[1], true,
                    VEC3(0.8f, 0.1f, 0.2f),
                    VEC3(0.8f, 0.1f, 0.2f),
                    VEC3(1.0f, 1.0f, 1.0f), 32.0f);

    material_create(&materials[2], false,
                    VEC3(0.3f, 0.2f, 0.8f),
                    VEC3(0.3f, 0.2f, 0.8f),
                    VEC3(0.8f, 0.8f, 0.8f), 32.0f);

    material_create(&materials[3], false,
                    VEC3(0.8f, 0.2f, 0.3f),
                    VEC3(0.8f, 0.2f, 0.3f),
                    VEC3(1.0f, 1.0f, 1.0f), 32.0f);

    material_create(&materials[4], true,
                    VEC3(0.8f, 0.1f, 0.2f),
                    VEC3(0.8f, 0.1f, 0.2f),
                    VEC3(1.0f, 1.0f, 1.0f), 32.0f);

    /* creating transforms */

    for(i32 i = 0; i < 6; i++) transform_reset(&transforms[i]);

    transforms[0].scale = VEC3(2.0f, 2.0f, 2.0f);

    transforms[1].pos = VEC3(5.0f, 0.0f, -2.0f);
    transforms[1].scale = VEC3(2.0f, 2.0f, 2.0f);

    // transform[2] has default transform

    transforms[3].pos = VEC3(3.0f, 0.0f, 0.0f);
    transforms[3].scale = VEC3(0.5f, 0.5f, 0.5f);

    transforms[4].pos = VEC3(-2.0f, 2.0f, 0.0f);

    transforms[5].pos = VEC3(-8.0f, 5.0f, 3.0f);
    transforms[5].scale = VEC3(4.0f, 4.0f, 4.0f);

    /* creating models */

    shapes_uv_sphere(&models[0], arena, 20, &materials[0], sphere_shader);
    shapes_uv_sphere(&models[1], arena, 20, &materials[1], sphere_shader);

    shapes_plane(&models[2], arena, 50.0f, 50.0f, 2, &materials[2], model_shader);
    model_load(&models[3], arena, "res/chicken/chicken.obj", model_shader);
    shapes_box(&models[4], arena, 1.5f, 2.0f, 3.0f, &materials[3], model_shader);
    shapes_uv_sphere(&models[5], arena, 20, &materials[4], sphere_shader);

    /* setting/updating transforms */

    for(i32 i = 0; i < 6; i++) model_update_transform(&models[i], &transforms[i]);

    /* adding models to scene */

    scene_add_model(&s.scenes[0], &models[0]);
    scene_add_model(&s.scenes[0], &models[1]);

    scene_add_model(&s.scenes[1], &models[2]);
    scene_add_model(&s.scenes[1], &models[3]);
    scene_add_model(&s.scenes[1], &models[4]);
    scene_add_model(&s.scenes[1], &models[5]);
}

void game_add_lights(Arena* arena)
{
    Light lights[3];
    DirLight dir_light;

    /* creating lights */

    light_create(&lights[0], VEC3(0.0f, 5.0f, 5.0f),      // position
                             VEC3(0.2f, 0.2f, 0.2f),      // ambient
                             VEC3(0.6f, 0.6f, 0.6f),      // diffuse
                             VEC3(0.8f, 0.8f, 0.8f),      // specular
                             VEC3(0.003f, 0.007f, 1.0f)); // attenuation constants (quadratic, linear, const)

    light_create(&lights[1], VEC3(10.0f, 5.0f, 5.0f),
                             VEC3(0.0f, 0.0f, 0.2f),
                             VEC3(0.0f, 0.0f, 0.6f),
                             VEC3(0.0f, 0.0f, 0.8f),
                             VEC3(0.007f, 0.014f, 1.0f));

    light_create(&lights[2], VEC3(-5.0f, 5.0f, -5.0f),
                             VEC3(0.2f, 0.0f, 0.0f),
                             VEC3(0.5f, 0.0f, 0.0f),
                             VEC3(0.8f, 0.0f, 0.0f),
                             VEC3(0.007f, 0.014f, 1.0f));

    dir_light_create(&dir_light, VEC3(-4.0f, -12.0f, 10.0f), // roughly the direction of the sun on the skybox
                                 VEC3(0.2f, 0.2f, 0.2f),
                                 VEC3(0.5f, 0.5f, 0.5f),
                                 VEC3(0.8f, 0.8f, 0.8f));

    /* creating a custom model for one of the lights */

    Model model;
    Transform transform;

    shapes_box(&model, arena, 1.0f, 1.0f, 1.0f, NULL, 0); // last 2 parameters are not used for lights, set to NULL and 0

    transform_reset(&transform);
    transform.euler = VEC3(45.0f, 30.0f, 45.0f); // can set euler and scale, but position will be set by scene_add_light
    transform.scale = VEC3(0.5f, 0.5f, 0.5f);
    model_update_transform(&model, &transform);

    /* adding lights to scene */

    scene_add_light(&s.scenes[0], arena, &s.rd, &lights[0], NULL);

    scene_add_light(&s.scenes[1], arena, &s.rd, &lights[1], NULL);
    scene_add_light(&s.scenes[1], NULL, &s.rd, &lights[2], &model);
    scene_set_dir_light(&s.scenes[1], &dir_light);

    /* update lights */

    for(u32 i = 0; i < s.scene_count; i++)
    {
        scene_update_light_data(&s.scenes[i]); // update light data without updating graphically, scene_switch() will do that
    }
}
