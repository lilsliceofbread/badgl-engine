#ifndef CAMERA_H
#define CAMERA_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

// forward declarations

typedef struct Camera
{
    vec3 pos;
    vec3 dir; // direction from cam - pos + dir = target

    vec3 up; // up perpendicularly to cam
    vec3 right; // right perpendicularly to cam

    float pitch, yaw;
    float last_cursor_x, last_cursor_y;

    mat4 view; // view matrix
} Camera;

Camera camera_init(GLFWwindow* window, void(*mouse_callback)(GLFWwindow* window, double cursor_x, double cursor_y));

void camera_calc_view(Camera* cam); // recalculate up/right vecs and view matrix based on pos/dir

#endif