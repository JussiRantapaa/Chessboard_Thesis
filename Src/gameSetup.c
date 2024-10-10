#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "gameLogic.h"

void init_board(char** board, Gamestate* gamestate){
    //Initialize game variables
    gamestate->kingpos[WHITE] = 4;
    gamestate->kingpos[BLACK] = 60;
    gamestate->piece_count[WHITE] = 16;
    gamestate->piece_count[BLACK] = 16;
    gamestate->king_moves = 3;
    gamestate->rook_moves = 15;
    gamestate->game_over = 0;
    gamestate->turn = 1;
    gamestate->move_number = 0;
    // Initialize piece positions
    for(int i = 0; i < 16; i++){
        gamestate->pieces[WHITE][i] = i;
        gamestate->pieces[BLACK][i] = 48 + i;
    }

    char init_vals[8][8] = {
        {'R','N','B','Q','K','B','N','R'},
        {'P','P','P','P','P','P','P','P'},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {'p','p','p','p','p','p','p','p'},
        {'r','n','b','q','k','b','n','r'}
    };
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
           board[i][j] = init_vals[i][j];
        }
    }
}
void display_winner(char** board,Gamestate* gamestate){

}

