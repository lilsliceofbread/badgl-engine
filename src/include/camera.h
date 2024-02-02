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

    float pitch, yaw;
    double last_cursor_x, last_cursor_y;
    float speed;
    float sensitivity;

    float fov;
    float aspect_ratio;
    float znear, zfar;
    mat4 view;
    mat4 proj;
} Camera;

Camera camera_create(vec3 start_pos, float start_pitch, float start_yaw, float speed, float sensitivity);

void camera_update(Camera* self, Renderer* rd);

void camera_update_view(Camera* self); // recalculate up/right vecs and view matrix based on pos/dir

void camera_update_proj(Camera* self, float fov, float aspect_ratio, float znear, float zfar);


#endif