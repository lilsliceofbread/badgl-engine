#include "game.h"

int main(int argc, char* argv[])
{
    GameState state;
    
    game_init(&state);
    
    float last_time = 0.0f, delta_time = 0.0f;
    while(!window_should_close(&(state.rd.win)))
    {
        float curr_time = (float)glfwGetTime();
        delta_time = curr_time - last_time;
        last_time = curr_time; 

        game_update(&state, curr_time, delta_time);

        window_end_frame(&(state.rd.win));
    }
    
    game_end(&state);
    return 0;
}