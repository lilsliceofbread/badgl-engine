#ifndef BADGL_CAMERA_H
#define BADGL_CAMERA_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "glmath.h"

#include "renderer.h"

// forward declarations
#undef bool
#define bool _Bool

typedef struct Camera
{
    vec3 pos;
    vec3 dir; // direction from cam - pos + dir = target

    // don't need up in cam (not useful for movement)
    vec3 right; // right perpendicularly to cam

    float pitch, yaw;
    float last_cursor_x, last_cursor_y;
    bool wait_update;

    mat4 view;
    mat4 proj; // view and projection matrix
} Camera;

Camera camera_init();

void camera_update(Camera* self, Renderer* rd, float delta_time);

void camera_calc_view_vecs(Camera* self); // recalculate up/right vecs and view matrix based on pos/dir

void camera_calc_proj(Camera* self, float fov, float aspect_ratio, float znear, float zfar);


#endif