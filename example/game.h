#ifndef BADGL_GAME_H
#define BADGL_GAME_H

#include "renderer.h"
#include "model.h"
#include "camera.h"
#include "skybox.h"
#include "sphere.h"

#define MAX_MODELS 5

typedef struct GameState
{
    Renderer rd;
    Camera cam;

    Model models[MAX_MODELS];
    uint32_t model_count;
    Sphere sphere;
    Skybox skybox;
} GameState;

void game_init(GameState* s);

void game_update(GameState* s);

void game_end(GameState* s);

#endif