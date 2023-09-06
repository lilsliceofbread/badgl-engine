#include <stdio.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

int main(int argc, char* argv[])
{
    GLFWwindow* window;

    if(!glfwInit())
    {
        fprintf(stderr, "ERR: failed to init GLFW");
        return -1;
    }    

    window = glfwCreateWindow(1280, 720, "learnopengl", NULL, NULL);
    if(!window)
    {
        fprintf(stderr, "ERR: failed to open window");
        return -1;
    }

    while(!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}