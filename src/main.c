#include <stdio.h>
#include "window.h"

int main(int argc, char* argv[])
{
    GLFWwindow* window = NULL;
    WinContext ctx; 
    ctx.width = 1680;
    ctx.height = 1050;
    
    window = window_init(&ctx);
    if(window == NULL)
    {
        glfwTerminate();
        return -1;
    }

    window_loop(window, &ctx);

    glfwTerminate();
    return 0;
}