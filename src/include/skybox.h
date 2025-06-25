#ifndef BGL_SKYBOX_H
#define BGL_SKYBOX_H

#include "texture.h"
#include "mesh.h"
#include "camera.h"
#include "model.h"
#include "renderer.h"

/**
 * @note the cubemap path and image/s must have specific format; see texture_cubemap_create for details 
 */
void skybox_create(Model* self, Arena* scratch, Renderer* rd, const char* cubemap_path);

void skybox_draw(Model* self, Renderer* rd, Camera* cam);

void skybox_free(Model* self);

#endif
