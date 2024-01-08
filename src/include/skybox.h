#ifndef BADGL_SKYBOX_H
#define BADGL_SKYBOX_H

#include "texture.h"
#include "mesh.h"
#include "camera.h"
#include "model.h"
#include "renderer.h"

Model skybox_init(Renderer* rd, const char* cubemap_path);

void skybox_draw(Model* self, Renderer* rd, Camera* cam);

void skybox_free(Model* self);

#endif