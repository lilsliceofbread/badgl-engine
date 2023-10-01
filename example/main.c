#include "game.h"

int main(void)
{
    GameState state;
    
    game_init(&state);
    
    while(!rd_win_should_close(&state.rd))
    {
        rd_begin_frame(&state.rd);

        game_update(&state);

        rd_end_frame(&state.rd);
    }
    
    game_end(&state);
    return 0;
}