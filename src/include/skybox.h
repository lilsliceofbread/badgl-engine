#ifndef BADGL_SKYBOX_H
#define BADGL_SKYBOX_H

#include "texture.h"
#include "mesh.h"
#include "camera.h"

typedef struct Skybox
{
    Texture texture;
    Mesh mesh;
} Skybox;

Skybox skybox_init(const char* cubemap_path);

void skybox_draw(Skybox* self, Renderer* rd, Camera* cam);

void skybox_free(Skybox* self);

#endif