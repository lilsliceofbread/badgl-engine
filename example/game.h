#ifndef BADGL_GAME_H
#define BADGL_GAME_H

#include "renderer.h"
#include "scene.h"

#define MAX_SCENES 5

typedef struct GameState
{
    Renderer rd;
    Scene scenes[MAX_SCENES];
    uint32_t current_scene;
    uint32_t scene_count;
} GameState;

void game_init(GameState* s);

void game_update(GameState* s);

void game_end(GameState* s);

#endif