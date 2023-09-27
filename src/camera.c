#include "camera.h"
#include <math.h>
#include <stdbool.h>

#define SENSITIVITY 0.1f
#define CAM_SPEED 2.0f

Camera camera_init()
{
    Camera self;
    self.yaw = -90.0f;
    self.pitch = 0.0f;
    self.last_cursor_x = 0.0f; // default value for starting
    self.last_cursor_y = 0.0f;
    self.wait_update = true;

    return self;
}

void camera_calc_view_vecs(Camera* self)
{
    vec3 dir = {
        cos(glm_rad(self->yaw)) * cos(glm_rad(self->pitch)), 
        sin(glm_rad(self->pitch)),
        sin(glm_rad(self->yaw)) * cos(glm_rad(self->pitch)), 
    };

    glm_vec3_normalize_to(dir, self->dir);

    glm_cross((vec3){0.0f, 1.0f, 0.0f}, self->dir, self->right);
    glm_vec3_normalize(self->right);
    glm_cross(self->dir, self->right, self->up); // may need to normalise
    glm_vec3_normalize(self->up);

    glm_mat4_identity(self->view);
    vec3 target;
    glm_vec3_add(self->pos, self->dir, target);
    glm_lookat(self->pos, target, self->up, self->view);
}

void camera_calc_proj(Camera* self, float fov, float aspect_ratio, float znear, float zfar)
{
    glm_mat4_identity(self->proj);
    glm_perspective(glm_rad(fov), aspect_ratio, znear, zfar, self->proj);
}

// MOVE TO PLAYER LATER, w/ vel, accel, player_pos
void camera_update(Camera* self, Renderer* rd, float delta_time)
{
    float self_step = CAM_SPEED * delta_time;
    vec3 step_vec;

    // remove y component from "velocity" vecs to keep moving on flat plane
    vec3 flat_dir, flat_right;
    glm_vec3_copy((vec3){self->dir[0], 0.0f, self->dir[2]}, flat_dir);
    glm_vec3_normalize(flat_dir);
    glm_vec3_copy((vec3){self->right[0], 0.0f, self->right[2]}, flat_right);
    glm_vec3_normalize(flat_right);

    if(rd_get_key(rd, GLFW_KEY_W))
    {
        glm_vec3_scale(flat_dir, self_step, step_vec);
        glm_vec3_add(self->pos, step_vec, self->pos);
    }
    if(rd_get_key(rd, GLFW_KEY_S))
    {
        glm_vec3_scale(flat_dir, self_step, step_vec);
        glm_vec3_sub(self->pos, step_vec, self->pos);
    }
    if(rd_get_key(rd, GLFW_KEY_A))
    {
        glm_vec3_scale(flat_right, self_step, step_vec);
        glm_vec3_add(self->pos, step_vec, self->pos);
    }
    if(rd_get_key(rd, GLFW_KEY_D))
    {
        glm_vec3_scale(flat_right, self_step, step_vec);
        glm_vec3_sub(self->pos, step_vec, self->pos);
    }
    if(rd_get_key(rd, GLFW_KEY_SPACE))
    {
        glm_vec3_scale((vec3){0.0f, 1.0f, 0.0f}, self_step, step_vec);
        glm_vec3_add(self->pos, step_vec, self->pos);
    }
    if(rd_get_key(rd, GLFW_KEY_LEFT_CONTROL))
    {
        glm_vec3_scale((vec3){0.0f, 1.0f, 0.0f}, self_step, step_vec);
        glm_vec3_sub(self->pos, step_vec, self->pos);
    }

    float cursor_x, cursor_y;
    rd_get_cursor_pos(rd, &cursor_x, &cursor_y);

    if(!rd->cursor_disabled || self->wait_update) // don't update rotation
    {
        self->last_cursor_x = cursor_x;
        self->last_cursor_y = cursor_y;
        self->wait_update = false;
        camera_calc_view_vecs(self);
        return;
    }

    self->yaw += SENSITIVITY * (cursor_x - self->last_cursor_x); // x offset * sens = yaw
    self->pitch += SENSITIVITY * (self->last_cursor_y - cursor_y); // y offset needs to be from bottom to top not top to bottom so -()
    self->last_cursor_x = cursor_x;
    self->last_cursor_y = cursor_y;

    // constrain pitch so you can't flip your "head"
    if(self->pitch > 89.0f)
    {
        self->pitch = 89.0f;
    } 
    else if(self->pitch < -89.0f)
    {
        self->pitch = -89.0f;
    }

    camera_calc_view_vecs(self);
}