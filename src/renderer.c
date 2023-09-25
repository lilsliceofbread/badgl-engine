#include "renderer.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "window.h"
#include "shader.h"
#include "texture.h"
#include "util.h"

void rd_init(Renderer* self, int width, int height)
{
    self->width = width;
    self->height = height;
    self->win = window_init(width, height);

#ifndef NO_DEBUG
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if(flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_callback, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
#endif

    glViewport(0, 0, self->width, self->height);

    self->cam = camera_init(); // should be user defined

    glEnable(GL_BLEND); // enable transparent textures
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // which blending function for transparency
    glEnable(GL_FRAMEBUFFER_SRGB); // use standard rgb for framebuffer
    //glDisable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // black
    
    shader_init(&(self->shader), "shaders/default.vert", "shaders/default.frag");

    float aspect_ratio = (float)(self->width) / (float)(self->height);
    camera_calc_proj(&(self->cam), 90.0f, aspect_ratio, 0.1f, 100.0f); // remove to user
}

/*void rd_add_texture(Renderer* self, GLuint texture_id, int texture_unit, const char* sampler_name)
{
    ASSERT(!(self->tex_count >= MAX_TEXTURES), "ERR: self max textures reached");
    self->textures[self->tex_count] = texture_id; 
    self->tex_count++;

    // tell sampler which texture unit to use
    rd_use_program(self);
    glUniform1i(glGetUniformLocation(self->shader_program, sampler_name), texture_unit);
        
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
}*/

void rd_send_vp_matrix(Renderer* self)
{
    shader_use(&(self->shader));
    mat4 vp;
    glm_mat4_mul(self->cam.proj, self->cam.view, vp);
    shader_uniform_mat4(&(self->shader), "vp", vp);
}

void rd_clear_frame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void rd_free(Renderer* self)
{
    shader_free(&(self->shader));
}

void rd_resize_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(window); 
    rd->width = width;
    rd->height = height;
    
    //printf("window resized - w: %d h: %d\n", width, height);
}