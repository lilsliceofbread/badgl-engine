#include "window.h"
#include <stdio.h>
#include <cglm/cglm.h>
#include "camera.h"
#include "renderer.h"

#define CAM_SPEED 2.0f

void process_inputs(GLFWwindow* window, WinContext* ctx, float delta_time) {
    Camera* cam = &(ctx->renderer.cam);
    float cam_step = CAM_SPEED * delta_time;
    vec3 step_vec;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1); // true
    }
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        glm_vec3_scale(cam->dir, cam_step, step_vec);
        glm_vec3_add(cam->pos, step_vec, cam->pos);
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        glm_vec3_scale(cam->dir, cam_step, step_vec);
        glm_vec3_sub(cam->pos, step_vec, cam->pos);
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        glm_vec3_scale(cam->right, cam_step, step_vec);
        glm_vec3_add(cam->pos, step_vec, cam->pos);
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        glm_vec3_scale(cam->right, cam_step, step_vec);
        glm_vec3_sub(cam->pos, step_vec, cam->pos);
    }
}

void test_loop(GLFWwindow* window, WinContext* ctx) {
    Renderer* renderer = &(ctx->renderer);
    GLuint shader_program = renderer->shader_program;
    GLuint vao = renderer->vao;
    GLuint vbo = renderer->vbo;
    GLuint ebo = renderer->ebo;
    GLuint texture1 = renderer->textures[0];

    GLint model_uniform = glGetUniformLocation(shader_program, "model");
    GLint view_uniform = glGetUniformLocation(shader_program, "view");
    GLint proj_uniform = glGetUniformLocation(shader_program, "proj");
    if(model_uniform == -1 || view_uniform == -1 || proj_uniform == -1)
    {
        fprintf(stderr, "ERR: could not find uniform");
        renderer_free(renderer); 
        return;
    }

    vec3 cube_pos[] = {
        {0.0f, 0.0f, 0.0f},
        {2.0f, 5.0f, -15.0f},
        {-1.5f, -2.2f, -2.5f},
        {-3.8f, -2.0f, -12.3f},
        {2.4f, -0.4f, -3.5f},
        {-1.7f, 3.0f, -7.5f},
        {1.3f, -2.0f, -2.5f},
        {1.5f, 2.0f, -2.5f},
        {1.5f, 0.2f, -1.5f},
        {-1.3f, 1.0f, -1.5f}
    };

    // cam starting position and angle
    Camera* cam = &(ctx->renderer.cam);
    glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, cam->pos);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam->dir); // looking down negative z

    mat4 proj;
    glm_mat4_identity(proj);
    float aspect_ratio = (float)(ctx->width)/(float)(ctx->height); // if don't cast both to float, will produce integer :(
    glm_perspective(glm_rad(90.0f), aspect_ratio, 0.1f, 100.0f, proj);
    glUseProgram(shader_program); // use program before sending projection mat to uniform
    glUniformMatrix4fv(proj_uniform, 1, false, (float*)proj);

    float last_time = 0.0f, delta_time = 0.0f;
    glClearColor(0.8f, 0.3f, 0.5f, 0.0f);
    while(!glfwWindowShouldClose(window))
    {
        float curr_time = (float)glfwGetTime();
        delta_time = curr_time - last_time;
        last_time = curr_time; 

        process_inputs(window, ctx, delta_time);

        float time = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        
        camera_calc_view(cam);

        glUseProgram(shader_program);

        glUniformMatrix4fv(view_uniform, 1, false, (float*)cam->view);

        mat4 model;
        glBindVertexArray(vao);
        for(int i = 0; i < 10; i++)
        {
            glm_mat4_identity(model);
            glm_translate(model, cube_pos[i]);

            vec3 axis; // translation becomes axis of rotation
            glm_vec3_add(cube_pos[i], (vec3){0.0f, 1.0f, 0.0f}, axis); // for initial cube with pos 0 0 0 don't have weirdness
            glm_vec3_normalize(axis);
            glm_rotate(model, time, axis);

            glUniformMatrix4fv(model_uniform, 1, false, (float*)model);
            //glDrawArrays(GL_TRIANGLES, 0, 36);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer_free(renderer);
}

int main(int argc, char* argv[])
{
    GLFWwindow* window = NULL;
    WinContext ctx; 
    ctx.width = 1280;
    ctx.height = 720;
    
    window = window_init(&ctx);
    if(window == NULL)
    {
        glfwTerminate();
        return -1;
    }

    test_loop(window, &ctx);

    glfwTerminate();
    return 0;
}