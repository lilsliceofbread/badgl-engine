#ifndef BADGL_GAME_H
#define BADGL_GAME_H

#include "renderer.h"
#include "model.h"
#include "camera.h"
#include "sphere.h"

#define MAX_MODELS 5

typedef struct GameState
{
    Renderer rd;
    int shader_index;
    Camera cam;
    Model models[MAX_MODELS];
    uint32_t model_count;
    Sphere sphere;
} GameState;

void game_init(GameState* s);

void game_update(GameState* s, float curr_time, float delta_time);

void game_end(GameState* s);

#endif