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

    camera_update_view(&self);

    return self;
}

void camera_update_view(Camera* self)
{
    vec3 world_up = {0.0f, 1.0f, 0.0f};

    vec3 dir = {
        cosf(math_rad(self->yaw)) * cosf(math_rad(self->pitch)), 
        sinf(math_rad(self->pitch)),
        sinf(math_rad(self->yaw)) * cosf(math_rad(self->pitch)), 
    };

    self->dir = dir;
    vec3_norm(&self->dir);

    self->right = vec_cross(self->dir, world_up);
    vec3_norm(&self->right);

    mat_look_at(&self->view, self->pos, self->dir, self->right);
}

void camera_update_proj(Camera* self, float fov, float aspect_ratio, float znear, float zfar)
{
    self->fov = fov;
    self->aspect_ratio = aspect_ratio;
    self->znear = znear;
    self->zfar = zfar;

    mat_perspective_fov(&self->proj, fov, aspect_ratio, znear, zfar);
    //self->proj = mat_perspective_frustrum(0.01f, 100.0f, -0.1f, 0.1f, -0.1f, 0.1f);
    //self->proj = mat_orthographic_frustrum(0.01f, 100.0f, -1.0f, 1.0f, -1.0f, 1.0f);
}

// MOVE TO PLAYER LATER, w/ vel, accel, player_pos
void camera_update(Camera* self, Renderer* rd)
{
    float cam_step = CAM_SPEED * rd->delta_time;

    vec3 step_vec;

    // remove y component from "velocity" vecs to keep moving on flat plane
    vec3 flat_dir = {
        self->dir.x,
        0.0f,
        self->dir.z
    };
    vec3_norm(&flat_dir);

    vec3 flat_right = {
        self->right.x,
        0.0f,
        self->right.z
    };
    vec3_norm(&flat_right);

    vec3 world_up = {0.0f, 1.0f, 0.0f};

    if(rd_key_pressed(rd, GLFW_KEY_W))
    {
        step_vec = vec3_scale(flat_dir, cam_step);
        self->pos = vec3_add(self->pos, step_vec);
    }
    if(rd_key_pressed(rd, GLFW_KEY_S))
    {
        step_vec = vec3_scale(flat_dir, cam_step);
        self->pos = vec3_sub(self->pos, step_vec);
    }
    if(rd_key_pressed(rd, GLFW_KEY_A))
    {
        step_vec = vec3_scale(flat_right, cam_step);
        self->pos = vec3_sub(self->pos, step_vec);
    }
    if(rd_key_pressed(rd, GLFW_KEY_D))
    {
        step_vec = vec3_scale(flat_right, cam_step);
        self->pos = vec3_add(self->pos, step_vec);
    }
    if(rd_key_pressed(rd, GLFW_KEY_SPACE))
    {
        step_vec = vec3_scale(world_up, cam_step);
        self->pos = vec3_add(self->pos, step_vec);
    }
    if(rd_key_pressed(rd, GLFW_KEY_LEFT_CONTROL))
    {
        step_vec = vec3_scale(world_up, cam_step);
        self->pos = vec3_sub(self->pos, step_vec);
    }

    float cursor_x, cursor_y;
    rd_get_cursor_pos(rd, &cursor_x, &cursor_y);

    if(!rd->cursor_disabled || rd->mouse_wait_update > 0) // don't update rotation
    {
        self->last_cursor_x = cursor_x;
        self->last_cursor_y = cursor_y;
        camera_update_view(self);
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

    camera_update_view(self);

    const float aspect_ratio = (float)(rd->width) / (float)(rd->height);
    if(aspect_ratio != self->aspect_ratio) // if aspect ratio has changed
    {
        camera_update_proj(self, self->fov, aspect_ratio, self->znear, self->zfar);
    }
}