#include "camera.h"
#include <math.h>

#define SENSITIVITY 0.1f
#define CAM_SPEED 2.0f

Camera camera_init()
{
    Camera cam;
    cam.yaw = -90.0f;
    cam.pitch = 0.0f;
    cam.last_cursor_x = 0.0f; // default value for starting
    cam.last_cursor_y = 0.0f;
    cam.wait_update = true; // should skip update for 1 frame

    return cam;
}

void camera_calc_view_vecs(Camera* cam)
{
    vec3 dir = {
        cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch)), 
        sin(glm_rad(cam->pitch)),
        sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch)), 
    };

    glm_vec3_normalize_to(dir, cam->dir);

    glm_cross((vec3){0.0f, 1.0f, 0.0f}, cam->dir, cam->right);
    glm_vec3_normalize(cam->right);
    glm_cross(cam->dir, cam->right, cam->up); // may need to normalise
    glm_vec3_normalize(cam->up);

    glm_mat4_identity(cam->view);
    vec3 target;
    glm_vec3_add(cam->pos, cam->dir, target);
    glm_lookat(cam->pos, target, cam->up, cam->view);
}

void camera_calc_proj(Camera* cam, float fov, float aspect_ratio, float znear, float zfar)
{
    glm_mat4_identity(cam->proj);
    glm_perspective(glm_rad(fov), aspect_ratio, znear, zfar, cam->proj);
}

void camera_mouse_update(Camera* cam, double cursor_x, double cursor_y)
{
    if(cam->wait_update) // if application just started
    {
        cam->last_cursor_x = cursor_x;
        cam->last_cursor_y = cursor_y;
        cam->wait_update = false;
        return;
    }

    cam->yaw += SENSITIVITY * (cursor_x - cam->last_cursor_x); // x offset * sens = yaw
    cam->pitch += SENSITIVITY * (cam->last_cursor_y - cursor_y); // y offset needs to be from bottom to top not top to bottom so -()
    cam->last_cursor_x = cursor_x;
    cam->last_cursor_y = cursor_y;

    // constrain pitch so you can't flip your "head"
    if(cam->pitch > 89.0f)
    {
        cam->pitch = 89.0f;
    } 
    else if(cam->pitch < -89.0f)
    {
        cam->pitch = -89.0f;
    }

    camera_calc_view_vecs(cam); 
}

// MOVE TO PLAYER LATER, w/ vel, accel, player_pos
void camera_process_inputs(Camera* cam, GLFWwindow* window, float delta_time)
{
    float cam_step = CAM_SPEED * delta_time;
    vec3 step_vec;
    
    // remove y component from "velocity" vecs to keep moving on flat plane
    vec3 flat_dir, flat_right;
    glm_vec3_copy((vec3){cam->dir[0], 0.0f, cam->dir[2]}, flat_dir);
    glm_vec3_normalize(flat_dir);
    glm_vec3_copy((vec3){cam->right[0], 0.0f, cam->right[2]}, flat_right);
    glm_vec3_normalize(flat_right);

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        glm_vec3_scale(flat_dir, cam_step, step_vec);
        glm_vec3_add(cam->pos, step_vec, cam->pos);
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        glm_vec3_scale(flat_dir, cam_step, step_vec);
        glm_vec3_sub(cam->pos, step_vec, cam->pos);
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        glm_vec3_scale(flat_right, cam_step, step_vec);
        glm_vec3_add(cam->pos, step_vec, cam->pos);
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        glm_vec3_scale(flat_right, cam_step, step_vec);
        glm_vec3_sub(cam->pos, step_vec, cam->pos);
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        glm_vec3_scale((vec3){0.0f, 1.0f, 0.0f}, cam_step, step_vec);
        glm_vec3_add(cam->pos, step_vec, cam->pos);
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        glm_vec3_scale((vec3){0.0f, 1.0f, 0.0f}, cam_step, step_vec);
        glm_vec3_sub(cam->pos, step_vec, cam->pos);
    }
    camera_calc_view_vecs(cam);
}