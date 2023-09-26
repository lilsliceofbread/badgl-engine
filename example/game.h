#ifndef BADGL_GAME_H
#define BADGL_GAME_H

#include "renderer.h"
#include "model.h"

#define MAX_MODELS 5

typedef struct GameState
{
    Renderer rd;
    Model models[MAX_MODELS];
    uint32_t model_count;
} GameState;

void game_init(GameState* state);

void game_update(GameState* state, float curr_time, float delta_time);

void game_end(GameState* state);

#endif