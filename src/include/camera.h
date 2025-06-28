#ifndef BGL_CAMERA_H
#define BGL_CAMERA_H

#include "bgl_math.h"
#include "window.h"

typedef struct Camera
{
    mat4 view;
    mat4 projection;

    vec3 pos;
    vec3 dir;   // direction from cam - pos + dir = target
                // don't need up in cam (not useful for movement)
    vec3 right; // right perpendicularly to cam

    f32 fov;
    f32 aspect_ratio;
    f32 znear, zfar;

    f32 pitch, yaw;
    f64 last_cursor_x, last_cursor_y;
    f32 speed;
    f32 sensitivity;
} Camera;

void camera_create(Camera* self, vec3 start_pos, f32 start_pitch, f32 start_yaw, f32 speed, f32 sensitivity);

void camera_update(Camera* self, BGLWindow* window, f32 delta_time);

void camera_update_view(Camera* self); // recalculate up/right vecs and view matrix based on pos/dir

void camera_update_proj(Camera* self, f32 fov, f32 aspect_ratio, f32 znear, f32 zfar);

#endif
