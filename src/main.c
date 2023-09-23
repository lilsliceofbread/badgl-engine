#include "window.h"
#include <cglm/cglm.h>
#include <stdio.h>
#include "renderer.h"
#include "camera.h"
#include "util.h"

void test_loop(GLFWwindow* window, Renderer* renderer);

int main(int argc, char* argv[])
{
    GLFWwindow* window;
    Renderer renderer;
    renderer.width = 1280;
    renderer.height = 720;
    
    window = window_init(&renderer);
    if(window == NULL)
    {
        glfwTerminate();
        return -1;
    }

    test_loop(window, &renderer);

    glfwTerminate();
    return 0;
}

void test_loop(GLFWwindow* window, Renderer* renderer) {
    // cam starting position and angle
    Camera* cam = &(renderer->cam);
    glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, cam->pos);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam->dir); // looking down negative z

    float last_time = 0.0f, delta_time = 0.0f;
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    while(!glfwWindowShouldClose(window))
    {
        float curr_time = (float)glfwGetTime();
        delta_time = curr_time - last_time;
        last_time = curr_time; 

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // will be player
        camera_process_inputs(cam, window, delta_time);

        renderer_use_program(renderer);
        mat4 vp;
        glm_mat4_mulN((mat4* []){&(cam->proj), &(cam->view)}, 2, vp);
        renderer_uniform_mat4(renderer, "vp", vp);

        mat4 model;
        glBindVertexArray(renderer->vao);
        glm_mat4_identity(model);
        glm_rotate(model, curr_time, (vec3){0.0f, 1.0f, 0.0f});

        renderer_uniform_mat4(renderer, "model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer_free(renderer);
}