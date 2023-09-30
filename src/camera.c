#include "camera.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define SENSITIVITY 0.1f
#define CAM_SPEED 5.0f

Camera camera_init(vec3 start_pos, float start_yaw, float start_pitch)
{
    Camera self;
    self.pos = start_pos;
    self.yaw = start_yaw;
    self.pitch = start_pitch;
    self.last_cursor_x = 0.0f; // default value for starting
    self.last_cursor_y = 0.0f;
    self.wait_update = true;

    return self;
}

void camera_calc_view_vecs(Camera* self)
{
    vec3 world_up = {0.0f, 1.0f, 0.0f};

    vec3 dir = {
        cosf(math_rad(self->yaw)) * cosf(math_rad(self->pitch)), 
        sinf(math_rad(self->pitch)),
        sinf(math_rad(self->yaw)) * cosf(math_rad(self->pitch)), 
    };

    self->dir = vec3_norm(dir);

    self->right = vec_cross(self->dir, world_up);
    self->right = vec3_norm(self->right);

    self->view = mat_look_at(self->pos, self->dir, self->right);
}

void camera_calc_proj(Camera* self, float fov, float aspect_ratio, float znear, float zfar)
{
    self->proj = mat_perspective_fov(fov, aspect_ratio, znear, zfar);
    //self->proj = mat_perspective_frustrum(0.01f, 100.0f, -0.1f, 0.1f, -0.1f, 0.1f);
    //self->proj = mat_orthographic_frustrum(0.01f, 100.0f, -1.0f, 1.0f, -1.0f, 1.0f);
}

// MOVE TO PLAYER LATER, w/ vel, accel, player_pos
void camera_update(Camera* self, Renderer* rd, float delta_time)
{
    float cam_step = CAM_SPEED * delta_time;

    vec3 step_vec;

    // remove y component from "velocity" vecs to keep moving on flat plane
    vec3 flat_dir = {
        self->dir.x,
        0.0f,
        self->dir.z
    };
    flat_dir = vec3_norm(flat_dir);

    vec3 flat_right = {
        self->right.x,
        0.0f,
        self->right.z
    };
    flat_right = vec3_norm(flat_right);

    vec3 world_up = {0.0f, 1.0f, 0.0f};

    if(rd_get_key(rd, GLFW_KEY_W))
    {
        step_vec = vec3_scale(flat_dir, cam_step);
        self->pos = vec3_add(self->pos, step_vec);
    }
    if(rd_get_key(rd, GLFW_KEY_S))
    {
        step_vec = vec3_scale(flat_dir, cam_step);
        self->pos = vec3_sub(self->pos, step_vec);
    }
    if(rd_get_key(rd, GLFW_KEY_A))
    {
        step_vec = vec3_scale(flat_right, cam_step);
        self->pos = vec3_sub(self->pos, step_vec);
    }
    if(rd_get_key(rd, GLFW_KEY_D))
    {
        step_vec = vec3_scale(flat_right, cam_step);
        self->pos = vec3_add(self->pos, step_vec);
    }
    if(rd_get_key(rd, GLFW_KEY_SPACE))
    {
        step_vec = vec3_scale(world_up, cam_step);
        self->pos = vec3_add(self->pos, step_vec);
    }
    if(rd_get_key(rd, GLFW_KEY_LEFT_CONTROL))
    {
        step_vec = vec3_scale(world_up, cam_step);
        self->pos = vec3_sub(self->pos, step_vec);
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

    //printf("CAMERA: pos: %f %f %f\n", self->pos.x, self->pos.y, self->pos.z);
    //printf("CAMERA: step: %f\n", cam_step);
    //printf("CAMERA: yaw: %f pitch: %f\n", self->yaw, self->pitch);

    camera_calc_view_vecs(self);
}