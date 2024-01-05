#ifndef BADGL_CAMERA_H
#define BADGL_CAMERA_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include "glmath.h"
#include "renderer.h"

typedef struct Camera
{
    vec3 pos;
    vec3 dir; // direction from cam - pos + dir = target

    // don't need up in cam (not useful for movement)
    vec3 right; // right perpendicularly to cam

    float pitch, yaw;
    float last_cursor_x, last_cursor_y;

    float fov;
    float aspect_ratio; // store aspect ratio to know if it has changed
    float znear, zfar;
    mat4 view;
    mat4 proj; // view and projection matrix
} Camera;

Camera camera_init(vec3 start_pos, float start_yaw, float start_pitch);

void camera_update(Camera* self, Renderer* rd);

void camera_update_view(Camera* self); // recalculate up/right vecs and view matrix based on pos/dir

void camera_update_proj(Camera* self, float fov, float aspect_ratio, float znear, float zfar);


#endif