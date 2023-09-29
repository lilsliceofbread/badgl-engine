#include "game.h"

int main()
{
    GameState state;
    
    game_init(&state);
    
    float last_time = 0.0f, delta_time = 0.0f;
    while(!rd_win_should_close(&(state.rd)))
    {
        float curr_time = (float)glfwGetTime();
        delta_time = curr_time - last_time;
        last_time = curr_time; 
        rd_begin_frame(&(state.rd));

        game_update(&state, curr_time, delta_time);

        rd_end_frame(&(state.rd));
    }
    
    game_end(&state);
    return 0;
}