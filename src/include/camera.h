#ifndef CAMERA_H
#define CAMERA_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "window.h"

// forward declarations
#undef bool
#define bool _Bool

typedef struct Camera
{
    bool wait_update;

    vec3 pos;
    vec3 dir; // direction from cam - pos + dir = target

    vec3 up; // up perpendicularly to cam
    vec3 right; // right perpendicularly to cam

    float pitch, yaw;
    float last_cursor_x, last_cursor_y;

    mat4 view, proj; // view and projection matrix
} Camera;

Camera camera_init();

void camera_mouse_update(Camera* self, double cursor_x, double cursor_y);

void camera_calc_view_vecs(Camera* self); // recalculate up/right vecs and view matrix based on pos/dir

void camera_calc_proj(Camera* self, float fov, float aspect_ratio, float znear, float zfar);

void camera_process_inputs(Camera* self, Window* win, float delta_time);

#endif