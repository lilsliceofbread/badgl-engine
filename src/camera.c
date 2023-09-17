#include "camera.h"
#include <math.h>

Camera camera_init(GLFWwindow* window, void(*mouse_callback)(GLFWwindow* window, double cursor_x, double cursor_y))
{
    Camera cam;
    cam.yaw = -90.0f;
    cam.pitch = 0.0f;
    cam.last_cursor_x = -1.0f; // default value for starting
    cam.last_cursor_y = -1.0f;
    glfwSetCursorPosCallback(window, mouse_callback);

    return cam;
}

void camera_calc_view(Camera* cam)
{
    glm_cross((vec3){0.0f, 1.0f, 0.0f}, cam->dir, cam->right);
    glm_vec3_normalize(cam->right);
    glm_cross(cam->dir, cam->right, cam->up); // may need to normalise
    glm_vec3_normalize(cam->up);

    glm_mat4_identity(cam->view);
    vec3 target;
    glm_vec3_add(cam->pos, cam->dir, target);
    glm_lookat(cam->pos, target, cam->up, cam->view);
}

