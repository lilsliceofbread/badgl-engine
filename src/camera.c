#include "camera.h"

#include <math.h>
#include <stdio.h>
#include "defines.h"
#include "bgl_math.h"

void camera_create(Camera* self, vec3 start_pos, f32 start_pitch, f32 start_yaw, f32 speed, f32 sensitivity)
{
    self->pos = start_pos;
    self->yaw = start_yaw;
    self->pitch = start_pitch;
    self->last_cursor_x = 0.0f; // default value for starting
    self->last_cursor_y = 0.0f; // might be why angle jumps when starting
    self->speed = speed;
    self->sensitivity = sensitivity;

    camera_update_view(self);
}

void camera_update_view(Camera* self)
{
    vec3 world_up = VEC3(0.0f, 1.0f, 0.0f);

    vec3 dir = VEC3( 
        cosf(RADIANS(self->yaw)) * cosf(RADIANS(self->pitch)),
        sinf(RADIANS(self->pitch)),
        sinf(RADIANS(self->yaw)) * cosf(RADIANS(self->pitch))
    );

    self->dir = dir;
    vec3_norm(&self->dir);

    self->right = vec_cross(self->dir, world_up);
    vec3_norm(&self->right);

    mat_look_at(&self->view, self->pos, self->dir, self->right);
}

void camera_update_proj(Camera* self, f32 fov, f32 aspect_ratio, f32 znear, f32 zfar)
{
    self->fov = fov;
    self->aspect_ratio = aspect_ratio;
    self->znear = znear;
    self->zfar = zfar;

    mat_perspective_fov(&self->projection, fov, aspect_ratio, znear, zfar);
}

void camera_update(Camera* self, BGLWindow* window, f32 delta_time)
{
    const f32 aspect_ratio = (f32)(window->width) / (f32)(window->height);
    if(aspect_ratio != self->aspect_ratio)
    {
        camera_update_proj(self, self->fov, aspect_ratio, self->znear, self->zfar);
    }

    const f32 cam_step = self->speed * (f32)delta_time;

    // remove y component from "velocity" vecs to keep moving on flat plane
    vec3 flat_dir = VEC3( 
        self->dir.x,
        0.0f,
        self->dir.z
    );
    vec3_norm(&flat_dir);

    vec3 flat_right = VEC3( 
        self->right.x,
        0.0f,
        self->right.z
    );
    vec3_norm(&flat_right);

    vec3 world_up = VEC3(0.0f, 1.0f, 0.0f);

    vec3 step_vec;
    if(window_key_pressed(window, GLFW_KEY_W))
    {
        step_vec = vec3_scale(flat_dir, cam_step);
        self->pos = vec3_add(self->pos, step_vec);
    }
    if(window_key_pressed(window, GLFW_KEY_S))
    {
        step_vec = vec3_scale(flat_dir, cam_step);
        self->pos = vec3_sub(self->pos, step_vec);
    }
    if(window_key_pressed(window, GLFW_KEY_A))
    {
        step_vec = vec3_scale(flat_right, cam_step);
        self->pos = vec3_sub(self->pos, step_vec);
    }
    if(window_key_pressed(window, GLFW_KEY_D))
    {
        step_vec = vec3_scale(flat_right, cam_step);
        self->pos = vec3_add(self->pos, step_vec);
    }
    if(window_key_pressed(window, GLFW_KEY_SPACE))
    {
        step_vec = vec3_scale(world_up, cam_step);
        self->pos = vec3_add(self->pos, step_vec);
    }
    if(window_key_pressed(window, GLFW_KEY_LEFT_CONTROL))
    {
        step_vec = vec3_scale(world_up, cam_step);
        self->pos = vec3_sub(self->pos, step_vec);
    }

    f64 cursor_x, cursor_y;
    bool mouse_enabled = window_get_cursor(window, &cursor_x, &cursor_y);

    if(!mouse_enabled)
    {
        self->last_cursor_x = cursor_x;
        self->last_cursor_y = cursor_y;
        camera_update_view(self);
        return;
    }

    self->yaw += self->sensitivity * (f32)(cursor_x - self->last_cursor_x); // x offset * sens = yaw
    self->pitch += self->sensitivity * (f32)(self->last_cursor_y - cursor_y); // reversed y offset
    self->last_cursor_x = cursor_x;
    self->last_cursor_y = cursor_y;

    self->pitch = CLAMP(self->pitch, -89.0f, 89.0f);

    camera_update_view(self);
}
