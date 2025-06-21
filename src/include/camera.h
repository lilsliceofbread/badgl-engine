#ifndef BGL_CAMERA_H
#define BGL_CAMERA_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "glmath.h"
#include "renderer.h"

typedef struct Camera
{
    vec3 pos;
    vec3 dir;   // direction from cam - pos + dir = target
                // don't need up in cam (not useful for movement)
    vec3 right; // right perpendicularly to cam

    f32 pitch, yaw;
    f64 last_cursor_x, last_cursor_y;
    f32 speed;
    f32 sensitivity;

    f32 fov;
    f32 aspect_ratio;
    f32 znear, zfar;
    mat4 view;
    mat4 proj;
} Camera;

Camera camera_create(vec3 start_pos, f32 start_pitch, f32 start_yaw, f32 speed, f32 sensitivity);

void camera_update(Camera* self, Renderer* rd);

void camera_update_view(Camera* self); // recalculate up/right vecs and view matrix based on pos/dir

void camera_update_proj(Camera* self, f32 fov, f32 aspect_ratio, f32 znear, f32 zfar);


#endif