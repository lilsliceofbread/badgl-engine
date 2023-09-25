#include "window.h"
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdbool.h>
#include "vao.h"
#include "renderer.h"
#include "camera.h"
#include "util.h"

#define GAME_WIDTH 1280
#define GAME_HEIGHT 720

void test_loop(Window* win, Renderer* rd);
void _mouse_callback(GLFWwindow* window, double cursor_x, double cursor_y);
void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

static bool cursor_disabled = true;

int main(int argc, char* argv[])
{
    Renderer rd;
    Window* win = &(rd.win);
    
    rd_init(&rd, GAME_WIDTH, GAME_HEIGHT);
    window_set_callback_ptr(win, &rd);
    window_key_callback(win, _key_callback);
    window_mouse_callback(win, _mouse_callback);
    printf("%s", glfwGetVersionString());

    test_loop(win, &rd);

    window_terminate();
    return 0;
}

void test_loop(Window* win, Renderer* rd) {
    // cam starting position and angle
    Camera* cam = &(rd->cam);
    glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, cam->pos);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, cam->dir); // looking down negative z

    float last_time = 0.0f, delta_time = 0.0f;
    while(!window_should_close(win))
    {
        float curr_time = (float)glfwGetTime();
        delta_time = curr_time - last_time;
        last_time = curr_time; 

        rd_clear_frame();

        // will be player
        camera_process_inputs(cam, win, delta_time);

        // multiply view and proj matrices, and send to uniform on shader
        rd_send_vp_matrix(rd);

        mat4 model;
        glm_mat4_identity(model);
        glm_rotate(model, curr_time, (vec3){0.0f, 1.0f, 0.0f});

        shader_uniform_mat4(&(rd->shader), "model", model);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        window_end_frame(win);
    }

    rd_free(rd);
}

void _mouse_callback(GLFWwindow* window, double cursor_x, double cursor_y)
{
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(window);
    Camera* cam = &(rd->cam);
    camera_mouse_update(cam, cursor_x, cursor_y);
}

void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Renderer* rd = (Renderer*)glfwGetWindowUserPointer(window);

    if(key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1); // true
    }
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        if(cursor_disabled) // toggle between cursor locked vs usable
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            rd->cam.wait_update = true; // to stop sharp turn when cursor jumps
            cursor_disabled = false;
        }
        else
        {
            glfwFocusWindow(window);
            glfwSetCursorPos(window, rd->width/2, rd->height/2); // move cursor to middle of screen
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            rd->cam.wait_update = true; // to stop sharp turn when cursor jumps
            cursor_disabled = true;
        }
    }
}
