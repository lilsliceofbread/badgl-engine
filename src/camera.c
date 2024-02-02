#include "camera.h"

#include <math.h>
#include <stdio.h>
#include "defines.h"
#include "glmath.h"

Camera camera_create(vec3 start_pos, float start_pitch, float start_yaw, float speed, float sensitivity)
{
    Camera self;
    self.pos = start_pos;
    self.yaw = start_yaw;
    self.pitch = start_pitch;
    self.last_cursor_x = 0.0f; // default value for starting
    self.last_cursor_y = 0.0f;
    self.speed = speed;
    self.sensitivity = sensitivity;

    camera_update_view(&self);

    return self;
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

void camera_update_proj(Camera* self, float fov, float aspect_ratio, float znear, float zfar)
{
    self->fov = fov;
    self->aspect_ratio = aspect_ratio;
    self->znear = znear;
    self->zfar = zfar;

    mat_perspective_fov(&self->proj, fov, aspect_ratio, znear, zfar);
    //mat_perspective_frustrum(&self->proj, 0.01f, 100.0f, -0.1f, 0.1f, -0.1f, 0.1f);
    //mat_orthographic_frustrum(&self->proj, 0.01f, 100.0f, -1.0f, 1.0f, -1.0f, 1.0f);
}

void camera_update(Camera* self, Renderer* rd)
{
    const float aspect_ratio = (float)(rd->width) / (float)(rd->height);
    if(aspect_ratio != self->aspect_ratio)
    {
        camera_update_proj(self, self->fov, aspect_ratio, self->znear, self->zfar);
    }

    const float cam_step = self->speed * (float)rd->delta_time;

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

    double cursor_x, cursor_y;
    rd_get_cursor_pos(rd, &cursor_x, &cursor_y);

    if(!rd->mouse_should_update)
    {
        self->last_cursor_x = cursor_x;
        self->last_cursor_y = cursor_y;
        camera_update_view(self);
        return;
    }

    self->yaw += self->sensitivity * (float)(cursor_x - self->last_cursor_x); // x offset * sens = yaw
    self->pitch += self->sensitivity * (float)(self->last_cursor_y - cursor_y); // reversed y offset
    self->last_cursor_x = cursor_x;
    self->last_cursor_y = cursor_y;

    self->pitch = CLAMP(self->pitch, -89.0f, 89.0f);

    camera_update_view(self);
}