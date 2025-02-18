#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "ws2812b.h"
#include "gameLogic.h"
#include "74HCT597.h"

uint8_t RESET_BUFFER[420] = {0};
uint64_t sensor_val = 0;
uint8_t piece_count = 0;
uint32_t MAIN_COLOR = MINT;

int main(void)
{
	uint8_t error_code = 0;

	uint8_t index = 0;

	TIM2_init();
	spi1_init();
	SPI2_init_8bit();
	shift_init();
	error_led_init();

	// Init LED pins and push button
	GPIO_init(GPIOD,12,OUTPUT,&error_code);
	GPIO_init(GPIOD,13,OUTPUT,&error_code);
	GPIO_init(GPIOD,15,OUTPUT,&error_code);
	GPIO_init(GPIOA,0,INPUT,&error_code);

	GPIO_init(GPIOB,1,INPUT,&error_code);
	GPIO_init(GPIOB,3,INPUT,&error_code);

	error_handler(&error_code);

	uint32_t buffer[NUM_LEDS];

	while(1){
	    // Initialize gamestate variables
	    Gamestate* gamestate = (Gamestate*)malloc(sizeof(Gamestate));
	    if(gamestate == NULL)
	        return EXIT_FAILURE;

	    // Create the board
	    char** board = (char**)malloc(8 * sizeof(char*));
	    if(board == NULL)
	        return EXIT_FAILURE;

	    for(int i = 0;i < 8;i++){
	        board[i] = (char*)malloc(8*sizeof(char));
	        if(board[i] == NULL){
	            for(int j = 0;j < i;j++){
	                free(board[j]);
	            }
	            return EXIT_FAILURE;
	        }
	    }
	    GAME_START:
	    // Initialize the board and game variables
	    init_board(board,gamestate);
	   // Keep everything red until the board is in start position
	    while(sensor_val != PIECE_STARTING_POS_MASK){
		    sensor_val = shift_read_64bit();
	    	set_color_according_to_val(RED,buffer,sensor_val);

			ws2812_transmit(buffer,NUM_LEDS);
			reset();

	    }
		// Dimm Leds and then wake them up
	    tim2_delay_ms(500);
	    sleep(RED,PIECE_STARTING_POS_MASK,buffer,20);
	    tim2_delay_ms(500);
	    wake_up(MINT,WHITE_STARTING_POS_MASK,buffer,20);

	    // Ask moves and display the board until game over
	    while(!(gamestate->game_over)){
	        gamestate->turn ^= 1;
	        uint64_t piece_positions = 0;
	        piece_count = gamestate->piece_count[gamestate->turn];
	        for(int i = 0; i < piece_count;i++){
	        	uint64_t curr_piece_pos = gamestate->pieces[gamestate->turn][i];
	        	piece_positions |= (1ULL<<curr_piece_pos);
	        }
	        uint32_t piece_color = (gamestate->turn == 0) ? MINT : RED;
	        set_color_according_to_val(piece_color,buffer,piece_positions);
			ws2812_transmit(buffer,NUM_LEDS);
			reset();
	        ask_move(board,gamestate,buffer);

	        if(gamestate->game_over){
	        	tim2_delay_ms(5000);
	        	BOARD_LAST_STATE = PIECE_STARTING_POS_MASK;
	        	goto GAME_START;
	        }
			sensor_val = shift_read_64bit();
	    }
	}
    return 0;
}


