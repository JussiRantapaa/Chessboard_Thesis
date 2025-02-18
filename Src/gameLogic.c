/*
    CORE MOVE GENERATION AND VALIDATION LOGIC
    1. Generate all legal moves and store them in a move list.
        1.1 Based on a pieces ruleset compute all of its moves
        1.2 If a move is within bounds we call the is_king_threatened function
        1.3 In is_king_threatened we make the move and loop all enemy pieces and see if they can attack the square the king is located
        1.4 If king is safe we add the move and continue looping
    2. Move information is stored into a 32bit integer as follows:
    - bits 0-5 starting square
    - bits 6-11 landing square
    - bits 12- store specific information of the move such as rookmoves,castling,capturing pieces etc..
    3. Ask user for input, hash it and see if it matches a move on the move list
        3.1 If we get a match check for bits 12- to see if the move contains special manouvers and handle them separately
        3.2A Make the move on the gameboard and update piece position
    4. repeat until move_count is 0 and then handle game ending
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "gameLogic.h"
#include <time.h>
#include "ws2812b.h"
#include "74HCT597.h"
#include <math.h>
#include "tim.h"
#include "spi.h"

uint32_t color1 = MINT;
uint32_t color2 = ORANGE;
uint64_t BOARD_LAST_STATE = PIECE_STARTING_POS_MASK;
uint64_t new_board_state = 0;
uint32_t new_hash = 0;
uint8_t starting_square = 0;
uint64_t current_board_state = 0;
uint64_t current_piece_index2 = 0;
uint64_t ending_square = 0;
uint64_t en_passant_index = 0;

void ask_move(char **board,Gamestate* gamestate,uint32_t buffer[]){
    int* move_list = (int*)malloc(sizeof(int) * 100);
    int move_count = 0;

    // Compute all the moves for a player and store them in the move list
    compute_moves(board,move_list,&move_count,gamestate);

    // If there are no legal moves check winning conditions
    if(!move_count){
       handle_game_ending(board,gamestate,buffer);
       return;
    }
    if(gamestate->turn == WHITE){
    	handle_user_move(board,gamestate,buffer,move_list,move_count);
    }else{
    	// Second delay for a better experience
    	tim2_delay_ms(1000);
    	handle_bot_move(board,gamestate,buffer,move_list,move_count);
    	tim2_delay_ms(200);
    }
    free(move_list);
}
void handle_game_ending(char **board,Gamestate* gamestate,uint32_t buffer[]){
	gamestate->game_over = 1;
	int turn = gamestate->turn;
	int row = decode(gamestate->kingpos[turn],ROW);
	int col = decode(gamestate->kingpos[turn],COL);

	// If king is threatened its checkmate
	if(is_king_threatened(row,col,row,col,board,KING_MOVE,gamestate)){
		uint64_t squares = (1ULL<<(encode(row,col)) | (1ULL<<(((gamestate->move_history[(gamestate->move_number) - 1])>>6) & 0x3F)));
		breathe(RED,squares,buffer,5,40);
	}else{
	// If king is not theatened its a stalemate
	// Loop all piece positions and breathe them orange to indicate stalemate
		
		// For white
		uint64_t piece_positions = 0;
		uint16_t piece_count = gamestate->piece_count[WHITE];
		for(int i = 0; i < piece_count;i++){
			uint64_t curr_piece_pos = gamestate->pieces[WHITE][i];
			piece_positions |= (1ULL<<curr_piece_pos);
		}
		// For black
		piece_count = gamestate->piece_count[BLACK];
		for(int i = 0; i < piece_count;i++){
			uint64_t curr_piece_pos = gamestate->pieces[BLACK][i];
			piece_positions |= (1ULL<<curr_piece_pos);
		}
		breathe(ORANGE,piece_positions,buffer,5,40);
	}

	ws2812_transmit(buffer,NUM_LEDS);
	reset();
}

// This is terrible I am so sorry
void handle_user_move(char **board,Gamestate* gamestate,uint32_t buffer[],int move_list[],int move_count){
   	int row,col;
	
	uint64_t board_start_state = BOARD_LAST_STATE;
	uint8_t move_made = ILLEGAL_MOVE;
	
	// Stay in the loop until a legal move is made
	while(move_made != LEGAL_MOVE){
		PICKING_UP_PIECE:
		new_board_state = board_start_state;
		
		// Wait till something changes on the board
		while(new_board_state == board_start_state){
			new_board_state = shift_read_64bit();
		}
		// XOR what piece was picked up
		uint64_t current_piece_index = (BOARD_LAST_STATE ^ new_board_state );
		
		// Get the index of the starting square
		starting_square = 0;
		while (current_piece_index >>= 1) {
		   starting_square++;
		}
	
	   // Get the status of the piece picked up
		row = decode(starting_square,ROW);
		col = decode(starting_square,COL);
		start = board[row][col];
		int piece_status = get_square_status(board,row,col,gamestate);
	
		// If its enemy piece light up square and wait until we are in the previous state
		if(piece_status == ENEMY_PIECE){
			set_color_according_to_val(RED,buffer,(1ULL<<starting_square));
			ws2812_transmit(buffer,NUM_LEDS);
			reset();
	
			 while(new_board_state != board_start_state){
				  new_board_state = shift_read_64bit();
			 }
			 set_color_according_to_val(0,buffer,(1ULL<<starting_square));
			 ws2812_transmit(buffer,NUM_LEDS);
			 reset();
			 // Go back to the beginning of making a move
			 goto PICKING_UP_PIECE;
	
		  // If it's a friendly piece continue
		}else{
			uint64_t possible_moves = moves_per_square(starting_square,move_list,move_count);
	
			// Light up possible moves
			set_color_according_to_val(MINT,buffer,possible_moves);
			add_color_according_to_val(ORANGE,buffer,(1ULL<<starting_square));
			ws2812_transmit(buffer,NUM_LEDS);
			reset();
	
			PLACE_PIECE:
			current_board_state = new_board_state;
	
			// Wait till the piece is placed somewhere
			while(current_board_state == new_board_state){
				current_board_state = shift_read_64bit();
			}
	
			// If the piece is placed back to the original square start the process again
			if(current_board_state == board_start_state){
				set_color_according_to_val(0,buffer,possible_moves);
				ws2812_transmit(buffer,NUM_LEDS);
				reset();
	
				goto PICKING_UP_PIECE;
			}
	
			// If its a capture move
			if(current_board_state < new_board_state){
				uint64_t eaten_piece_index = current_board_state ^ new_board_state;
				if(eaten_piece_index & possible_moves){
					uint64_t desired_board_state = new_board_state;
					// Wait till the piece gets put on the square we ate
					while(current_board_state != desired_board_state){
						current_board_state = shift_read_64bit();
						uint64_t faulty_squares = current_board_state ^ desired_board_state;
						add_color_according_to_val(RED,buffer,faulty_squares);
						ws2812_transmit(buffer,NUM_LEDS);
						reset();
					}
	
					current_piece_index2 = eaten_piece_index;
				// If the position is not legal wait till the board is in start state and start the turn over
				}else{
					set_color_according_to_val(BLUE,buffer,eaten_piece_index | (1ULL<<starting_square));
					ws2812_transmit(buffer,NUM_LEDS);
					reset();
					while(current_board_state != board_start_state){
						current_board_state = shift_read_64bit();
						uint64_t faulty_squares = current_board_state ^ new_board_state;
						set_color_according_to_val(RED,buffer,faulty_squares | (1ULL<<starting_square));
						ws2812_transmit(buffer,NUM_LEDS);
						reset();
					}
					set_color_according_to_val(0,buffer,(1ULL<<starting_square));
					ws2812_transmit(buffer,NUM_LEDS);
					goto PICKING_UP_PIECE;
	
				}
			// If we are not eating a piece	XOR where the piece was placed
			}else{
				current_piece_index2 = (current_board_state ^ new_board_state);
			}
	
			// Calculate index of the square
			ending_square = 0;
			while (current_piece_index2 > 0) {
				if(current_piece_index2 & 1)break;
				current_piece_index2 = (current_piece_index2>>1);
				ending_square++;
			}
	
			if((1ULL<<ending_square) & possible_moves){
				move_made = LEGAL_MOVE;
	
				uint32_t move_hash = starting_square | (ending_square<<6);
				for(int i = 0; i < move_count;i++){
					uint32_t real_hash = move_list[i] & 0xFFF;
					if(real_hash == move_hash){
						new_hash = move_list[i];
						make_move(board,new_hash,gamestate);
						BOARD_LAST_STATE = shift_read_64bit();
	
						gamestate->move_history[gamestate->move_number++] = new_hash;
						return;
					}
				}
	
			}else{
				add_color_according_to_val(RED,buffer,(1ULL<<ending_square));
				ws2812_transmit(buffer,NUM_LEDS);
				reset();
	
				while(current_board_state != new_board_state){
					current_board_state = shift_read_64bit();
				}
				goto PLACE_PIECE;
			}
		}
	}
}
void handle_bot_move(char **board,Gamestate* gamestate,uint32_t buffer[],int move_list[],int move_count){
	int move_index = 0;
	int capture_move_found = 0;
	for(int i = 0; i < move_count;i++){
		if(move_list[i] & CAPTURE_MOVE){
			move_index = i;
			capture_move_found = 1;
			break;
		}
	}
	if(!capture_move_found)
		move_index = get_random_index(move_count);


	uint32_t move_hash = move_list[move_index];
	uint8_t starting_square = (uint8_t)(move_hash & 0x3F);
	uint8_t landing_square = (uint8_t)((move_hash>>6) & 0x3F);

	uint64_t board_start_state = BOARD_LAST_STATE;

	set_color_according_to_val(ORANGE,buffer,(1ULL<<starting_square));
	add_color_according_to_val(PURPLE,buffer,(1ULL<<landing_square));
	ws2812_transmit(buffer,NUM_LEDS);
	reset();

	// Wait till piece is picked up
	uint64_t desired_board_state = board_start_state - (1ULL<<starting_square);
	uint64_t current_board_state = shift_read_64bit();

	while(current_board_state != desired_board_state){
		current_board_state = shift_read_64bit();
		uint64_t faulty_squares = (desired_board_state ^ current_board_state);
		set_color_according_to_val(PURPLE,buffer,(1ULL<<landing_square));
		add_color_according_to_val(RED,buffer,faulty_squares);
		add_color_according_to_val(ORANGE,buffer,(1ULL<<starting_square));
		ws2812_transmit(buffer,NUM_LEDS);
		reset();
	}
//	if(move_hash & EN_PASSANT_MOVE){
//		desired_board_state |= (1ULL<<landing_square);
//		current_board_state = shift_read_64bit();
//
//		while(current_board_state != desired_board_state){
//			current_board_state = shift_read_64bit();
//			uint64_t faulty_squares = (desired_board_state ^ current_board_state);
//
//			set_color_according_to_val(ORANGE,buffer,(1ULL<<starting_square));
//			add_color_according_to_val(RED,buffer,faulty_squares);
//			add_color_according_to_val(PURPLE,buffer,(1ULL<<landing_square));
//			ws2812_transmit(buffer,NUM_LEDS);
//			reset();
//		}
//	}
	if(move_hash & CAPTURE_MOVE && !(move_hash & EN_PASSANT_MOVE)){
		desired_board_state = current_board_state - (1ULL<<landing_square);
		current_board_state = shift_read_64bit();

		while(current_board_state != desired_board_state){
			current_board_state = shift_read_64bit();
			uint64_t faulty_squares = (desired_board_state ^ current_board_state);

			set_color_according_to_val(ORANGE,buffer,(1ULL<<starting_square));
			add_color_according_to_val(RED,buffer,faulty_squares);
			add_color_according_to_val(PURPLE,buffer,(1ULL<<landing_square));
			ws2812_transmit(buffer,NUM_LEDS);
			reset();
		}
	}
	desired_board_state = current_board_state + (1ULL<<landing_square);
	while(current_board_state != desired_board_state){
			current_board_state = shift_read_64bit();
			uint64_t faulty_squares = (desired_board_state ^ current_board_state);

			set_color_according_to_val(ORANGE,buffer,(1ULL<<starting_square));
			add_color_according_to_val(RED,buffer,faulty_squares);
			add_color_according_to_val(PURPLE,buffer,(1ULL<<landing_square));
			ws2812_transmit(buffer,NUM_LEDS);
			reset();
	}

	make_move(board,move_hash,gamestate);
	BOARD_LAST_STATE = shift_read_64bit();

	gamestate->move_history[gamestate->move_number++] = move_hash;

}
uint64_t moves_per_square(int square,int* move_list,int move_count){
    square = square & 0x3F;
    uint64_t moves = 0;
    for(int i = 0;i < move_count;i++){
        int curr = move_list[i] & 0x3F;
        if(curr == square){
        	uint8_t possible_move =  (move_list[i]>>6) & 0x3F;
        	moves |= (1ULL<< possible_move);
        }
    }
    return moves;
}
int get_random_index(int move_count){
    time_t t;
    srand((unsigned) time(&t));
    int index = rand() % move_count;

    return index;
}
// Returns the hash of a row and column
int encode(int row,int col){
    return ((row * 8 + col));
}
// Returns either the row or column of a hash
int decode(int hash, int choice){
    if(choice == ROW)
        return (hash & 0x3F ) / 8;
    else
        return (hash & 0x3F) % 8;
}
// Hash the input and check if it matches with a move in the move list
int test_move(char **board,int row,int col,int end_row,int end_col,int* move_count,int* move_list){
    // Bits 0-5 are for the starting square and 6-12 for the landing square
    int hash = encode(row,col) | (encode(end_row,end_col)<<6);
    for(int i = 0;i < *move_count;i++){
        if((move_list[i] & 0xFFF) == hash)
            return move_list[i] ;
    }
    // If no match return 0
    return 0;
}
void swap(int* a,int* b){
    int temp = *a;
    *a = *b;
    *b = temp;
}
// Make the move on the board and update game variables
void make_move(char** board,int hash,Gamestate* gamestate){
    int row = decode(hash,ROW);
    int col = decode(hash,COL);
    int end_row = decode(hash>>6,ROW);
    int end_col = decode(hash>>6,COL);
    int turn = gamestate->turn;
    board[end_row][end_col] = board[row][col];
    board[row][col] = EMPTY_SQUARE;
    // Check for special moves and update their respected variables
    if(hash & EN_PASSANT_MOVE){
		// Get the hash of the piece that got En passanted
		int dir = (turn == WHITE) ? -1 : 1;
		int en_passant_hash = encode(end_row+dir,end_col);
		TEST_VAL = en_passant_hash;
		uint64_t current_board_status = shift_read_64bit();
		// Wait till the en passanted piece is taken off the board
		while(current_board_status & (1ULL<<en_passant_hash)){
			current_board_status = shift_read_64bit();
		}

		// Find the index of the piece that got captured,swap it with the last index and decrement piece count
		swap(&gamestate->pieces[!turn][get_piece_index((en_passant_hash<<6),gamestate,ENEMY_PIECE)],&gamestate->pieces[!turn][gamestate->piece_count[!turn] - 1]);
		gamestate->piece_count[!turn]--;
		board[end_row + dir][end_col] = EMPTY_SQUARE;

    }if(hash & KING_MOVE){
            gamestate->king_moves &=~(1U<<turn);
            gamestate->kingpos[turn] = (hash>>6) & 0x3F;

    }if(hash & PAWN_MOVE && (end_row == 7 || end_row == 0)){
        // Handle pawn promotion
        board[end_row][end_col] = (turn == WHITE) ? 'Q' : 'q';

    }if(hash & CAPTURE_MOVE && !(hash & EN_PASSANT_MOVE)){
        int captured_piece_index = get_piece_index(hash,gamestate,ENEMY_PIECE);
        // This looks horrible but really we just find the index of the captured piece, swap it with the index of the last piece
        // And decrement the piece_count
        swap(&gamestate->pieces[!turn][captured_piece_index],&gamestate->pieces[!turn][gamestate->piece_count[!turn] - 1]);
        gamestate->piece_count[!turn]--;

    }if(hash & CASTLING_MOVE){
        make_castling_move(board,row,col,end_row,end_col,gamestate);
        gamestate->kingpos[turn] = (hash>>6) & 0x3F;

    }if(hash & ROOK_MOVE){
        int rook_bit = (col == 0) ? 0 : 2;
        // Update the rook_moves variable. Bit 0 = white A rook, bit 1 = black A rook, bit 2 = white h rook, bit 3 = black h rook
        gamestate->rook_moves &=~ (1U<<(turn + rook_bit));
    }
    // Update the position of the piece we moved
    for(int i = 0;i < gamestate->piece_count[turn];i++){
        if(gamestate->pieces[turn][i] == (hash & 0x3F)){
            gamestate->pieces[turn][i] = ((hash>>6) & 0x3F);
            return;
        }
    }
}
int get_piece_index(int hash,Gamestate* gamestate,int piece_color){
    // Piece_color determines which color pieces we access
    int color = gamestate->turn;
    if(piece_color == ENEMY_PIECE)
        color = !color;

    // Loop the piece array and return the index when we find a match
    for(int i = 0;i < gamestate->piece_count[color];i++){
        if((gamestate->pieces[color][i] & 0x3F) == ((hash>>6) & 0x3F)){
            return i;
        }
    }
   return EXIT_FAILURE;
}
void make_castling_move(char** board,int row,int col,int end_row,int end_col,Gamestate* gamestate){
    int rook_hash = 0;
    int new_rook_col = 0;
    int new_rook_pos = 0;
    // Based on the end_col we move the rook on the board and create it's hash so we can find it from the piece list
    // Queens side castling
    if(end_col == 2){
        board[row][3] = board[row][0];
        board[row][0] = EMPTY_SQUARE;
        rook_hash = encode(row,0);
        new_rook_col = 3;
    }else{
    // Kings side castling
        board[row][5] = board[row][7];
        board[row][7] = EMPTY_SQUARE;
        rook_hash = encode(row,7);
        new_rook_col = 5;
    }
    new_rook_pos = encode(row,new_rook_col);

    // Update rook position
    for(int i = 0; i < gamestate->piece_count[gamestate->turn];i++){
    	if(gamestate->pieces[gamestate->turn][i] == rook_hash){
    		gamestate->pieces[gamestate->turn][i] = new_rook_pos;
    	    TEST_VAL = gamestate->pieces[gamestate->turn][i];
    		break;
    	}
    }

//    int rook_index = get_piece_index(rook_hash,gamestate,FRIENDLY_PIECE);
//    // Update the rook position in the pieces array and the king moves variable
//    gamestate->pieces[gamestate->turn][rook_index] = new_rook_pos;
    gamestate->king_moves &=~ (1U<<(gamestate->turn));

   uint64_t current_board_status = shift_read_64bit();
   while((current_board_status & (1ULL<<rook_hash)) || !(current_board_status & (1ULL<<new_rook_pos))){
	   current_board_status = shift_read_64bit();
   }
   BOARD_LAST_STATE = current_board_status;
}
void add_move(int row,int col,int end_row,int end_col,int* moves,int* move_count,int info){
    // Hash the move and add it to the list
    // Bits 0-5 point the starting square and 6-12 the landing square
    int move = (row * 8 + col) | ((end_row * 8 + end_col)<<6) | info;
    moves[*move_count] = move;
    (*move_count)++;
}
// Returns the status of a square on the gameboard
int get_square_status(char** board,int row, int col,Gamestate* gamestate){
    int turn = gamestate->turn;
    int piece = board[row][col];
    if((turn == WHITE && piece > WHITE_PIECES) || (turn == BLACK && piece < WHITE_PIECES && piece != EMPTY_SQUARE))
        return ENEMY_PIECE;
    else if(piece == EMPTY_SQUARE)
        return EMPTY_SQUARE;
    else
        return FRIENDLY_PIECE;
}

/****RAY FUNCTIONS****
    -Each function uses a ray to test if it can attack a specific square on the board
    -These functions are used when testing king threats*/
int vertical_ray(int row,int col,int new_row,int new_col,char** board){
    int dir = (new_row > row) ? 1 : -1;
    row += dir;
    //nIf we are not in same column early exit
    if(col != new_col)
        return 0;
    // Check the status of all squares in between, if we hit something return 0
    while(row != new_row && row < 8 && row >= 0){
        if(board[row][col] != EMPTY_SQUARE)
            return 0;
        row += dir;
    }
    // Return 1 if we are in the target square
    if(row == new_row)
        return 1;
    return 0;
}
int horizontal_ray(int row,int col,int new_row,int new_col,char** board){
    int dir = (new_col > col) ? 1 : -1;
    col += dir;
    // If not we are not in the same row early exit
    if(row != new_row)
        return 0;
    // Check the status of all squares in between, if we hit something return 1
    while(col != new_col && col < 8 && col >= 0){
        if(board[row][col] != EMPTY_SQUARE)
            return 0;
        col += dir;
    }
    // Return 1 if we are in the target square
    if(col == new_col)
        return 1;
    return 0;
}
int diagonal_ray(int row,int col,int new_row,int new_col,char** board){
    int row_dir = (new_row > row) ? 1 : -1;
    int col_dir = (new_col > col) ? 1 : -1;
    col += col_dir;
    row += row_dir;
    // If we land in the same row or column we stop looping or if we hit a piece we can exit
    while(col != new_col && row != new_row && col < 8 && col >= 0 && row < 8 && row  >= 0 ){
        if(board[row][col] != EMPTY_SQUARE){
            return 0;
        }
        col += col_dir;
        row += row_dir;
    }
    // If we get into target square we have a hit
    if(row == new_row && col == new_col)
        return 1;
    return 0;
}
int test_rook_threat(int row,int col,int new_row,int new_col,char** board){
    return vertical_ray(row,col,new_row,new_col,board) || horizontal_ray(row,col,new_row,new_col,board);
}
int test_bishop_threat(int row,int col,int new_row,int new_col,char** board){
    return diagonal_ray(row,col,new_row,new_col,board);
}
int test_queen_threat(int row,int col,int new_row,int new_col,char** board){
    return test_bishop_threat(row,col,new_row,new_col,board) || test_rook_threat(row,col,new_row,new_col,board);
}
int test_pawn_threat(int row,int col,int new_row,int new_col,char** board,Gamestate* gamestate){
    int dir = (gamestate->turn == WHITE) ? -1 : 1;
    // Check one square diagonally for both directions
    if(new_row == row + dir && (new_col == col + 1 || new_col == col - 1))
        return 1;
    return 0;
}
int test_knight_threat(int row,int col,int king_row,int king_col,char** board){
    int row_distance = abs(king_row - row);
    int col_distance = abs(king_col - col);
    // if distance is 1 & 2 or 2 & 1 we have a hit
    if((row_distance == 1 && col_distance == 2) || (row_distance == 2 && col_distance == 1))
        return 1;
    return 0;
}
int test_king_threat(int row,int col,int king_row,int king_col,char** board){
    int row_distance = abs(king_row - row);
    int col_distance = abs(king_col - col);
    // If distance is 1 in any direction we have a hit
    if(row_distance <= 1 && col_distance <= 1)
        return 1;
    return 0;
}
// Check if king is threatened by a move 0 = no threat, 1 = threat
int is_king_threatened(int row,int col,int new_row,int new_col,char** board,int move_info,Gamestate* gamestate){
    // If a move is valid according to the pieces ruleset, we still have to ensure it doesn't endanger our king
    // We make the move under test on the board, run through enemy pieces and see if our king is in danger
    // Finally we restore the board to it's original state

    // Get our kings coordinates from the gamestate variable
    int turn = gamestate->turn;
    int king_row = decode(gamestate->kingpos[turn],ROW);
    int king_col = decode(gamestate->kingpos[turn],COL);

    // En passant stuff
    int dir = (turn == WHITE) ? -1 : 1;
    char en_passant_piece = EMPTY_SQUARE;

    // If we are moving the king, we have to refererence the new squares
    if(move_info == KING_MOVE){
       king_row = new_row;
       king_col = new_col;
    }else if(move_info == EN_PASSANT_MOVE){
        // If we en passant, the piece that gets EN PASSANTE'D has to be stored
        en_passant_piece = board[row - dir][col];
        board[row - dir][col] = EMPTY_SQUARE;
    }
    int threat = 0;
    // Pretend to make the move on the board and store the information of the square we are attacking
    char previous_piece = board[new_row][new_col];
    board[new_row][new_col] = board[row][col];
    board[row][col] = EMPTY_SQUARE;

    int enemy_pieces = (turn == WHITE) ? 1 : 0;

    // Run through the enemy pieces on the board
    for(int i = 0; i < gamestate->piece_count[enemy_pieces];i++){
        int enemy_row = decode(gamestate->pieces[enemy_pieces][i],ROW);
        int enemy_col = decode(gamestate->pieces[enemy_pieces][i],COL);
        if(enemy_col == new_col && enemy_row == new_row){
            // If we are testing the piece that got eaten we do nothing
        }else{
            char piece = board[enemy_row][enemy_col];

            if(piece == 'R' || piece == 'r')
                threat = test_rook_threat(enemy_row,enemy_col,king_row,king_col,board);
            else if(piece == 'B' || piece == 'b')
                threat = test_bishop_threat(enemy_row,enemy_col,king_row,king_col,board);
            else if(piece == 'Q' || piece == 'q')
                threat = test_queen_threat(enemy_row,enemy_col,king_row,king_col,board);
            else if(piece == 'P' || piece == 'p')
                threat = test_pawn_threat(enemy_row,enemy_col,king_row,king_col,board,gamestate);
            else if(piece == 'N' || piece == 'n')
                threat = test_knight_threat(enemy_row,enemy_col,king_row,king_col,board);
            else
                threat = test_king_threat(enemy_row,enemy_col,king_row,king_col,board);
        }
        if(threat){
            goto EXIT_LOOP;
        }
    }
    EXIT_LOOP:
    // Place the board to its original position
    board[row][col] = board[new_row][new_col];
    board[new_row][new_col] = previous_piece;

    if(move_info == EN_PASSANT_MOVE)
        board[row - dir][col] = en_passant_piece;

    return threat;
}
// Computes all the moves for a said piece, this function is only called once every turn so the time lost by looping is not
// that expensive
void compute_moves(char** board,int* moves,int* move_count,Gamestate* gamestate){
    for(int i = 0;i < 8;i++){
        for(int j = 0;j < 8;j++){
            int current_square = get_square_status(board,i,j,gamestate);
            if(current_square == FRIENDLY_PIECE){
                int current_piece = board[i][j];
                current_piece = (current_piece >= 'A' && current_piece <= 'Z') ? current_piece + ('a'-'A'):current_piece;
                enum pieces piece = current_piece;
                switch (piece){
                case PAWN:
                   compute_pawn_moves(i,j,board,moves,move_count,gamestate);
                    break;
                case ROOK:
                    compute_rook_moves(i,j,board,moves,move_count,gamestate);
                    break;
                case KNIGHT:
                   compute_knight_moves(i,j,board,moves,move_count,gamestate);
                    break;
                case BISHOP:
                    compute_bishop_moves(i,j,board,moves,move_count,gamestate);
                    break;
                case QUEEN:
                    compute_queen_moves(i,j,board,moves,move_count,gamestate);
                    break;
                case KING:
                    compute_king_moves(i,j,board,moves,move_count,gamestate);
                    break;
                }
            }
        }
    }
}
void compute_pawn_moves(int row,int col,char** board,int* moves, int* move_count,Gamestate* gamestate){
    compute_forward_moves(row,col,board,moves,move_count,gamestate);
    compute_capture_moves(row,col,board,moves,move_count,gamestate);
    compute_en_passant_moves(row,col,board,moves,move_count,gamestate);
}
void compute_forward_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate){
    int turn = gamestate->turn;
    int first_move = ((turn == WHITE && row == 1) || (turn == BLACK && row == 6)) ? 1 : 0;
    int move_dir = (turn == WHITE) ? 1 : -1;
    // Single row moves
    if(board[row + move_dir][col] == EMPTY_SQUARE  && !is_king_threatened(row,col,row + move_dir,col,board,0,gamestate)){
        add_move(row,col,row + move_dir,col,moves,move_count,PAWN_MOVE);
     // Two row moves
        if(first_move && board[row + move_dir*2][col] == EMPTY_SQUARE && !is_king_threatened(row,col,row + move_dir*2,col,board,0,gamestate))
            add_move(row,col,row + move_dir*2,col,moves,move_count,PAWN_DOUBLE_MOVE | PAWN_MOVE);
    }
}
void compute_capture_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate){
    int turn = gamestate->turn;
    int move_dir = (turn == WHITE) ? 1 : -1;
    int left_col = col - 1;
    int right_col = col + 1;
    // Test left diagonal
    if(left_col < 8 && left_col >=0){
        int left_piece = get_square_status(board,row + move_dir,left_col,gamestate);
        if(left_piece == ENEMY_PIECE && !is_king_threatened(row,col,row + move_dir,left_col,board,0,gamestate)){

            add_move(row,col,row + move_dir,left_col,moves,move_count,CAPTURE_MOVE | PAWN_MOVE);
        }
    // Test right diagonal
    }if(right_col < 8 && right_col >= 0){
        int right_piece = get_square_status(board,row + move_dir,right_col,gamestate);
        if((right_piece == ENEMY_PIECE && !is_king_threatened(row,col,row + move_dir,right_col,board,0,gamestate))){

            add_move(row,col,row + move_dir,right_col,moves,move_count,CAPTURE_MOVE | PAWN_MOVE);
        }
    }
}
void compute_en_passant_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate){
    // If the last move was not a double pawn move, early exit
    if(!(gamestate->move_history[gamestate->move_number-1] & PAWN_DOUBLE_MOVE)){
        return;
    }
    int prev_move_row = decode((gamestate->move_history[gamestate->move_number-1])>>6,ROW);
    int prev_move_col = decode((gamestate->move_history[gamestate->move_number-1])>>6,COL);
    int turn = gamestate->turn;
    int dir = (turn == WHITE) ? 1 : -1;
    // If previous move was a pawn double move and the column is adjacent to ours its legal
    if(abs(prev_move_col - col) == 1 && prev_move_row == row &&
        !is_king_threatened(row,col,row + dir,prev_move_col,board,EN_PASSANT_MOVE,gamestate)){
        add_move(row,col,row + dir,prev_move_col,moves,move_count,EN_PASSANT_MOVE | PAWN_MOVE | CAPTURE_MOVE);
    }
}
void compute_knight_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate){
    // We can get every direction with two loops that go from negative to positive
    for(int dir1 = -1; dir1 <= 1;dir1 +=2){
        for(int dir2 = -1; dir2 <= 1;dir2 +=2){
            int new_row = row + 2 * dir1;
            int new_col = col + 1 * dir2;
            if(new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8){
                int current_square = get_square_status(board,new_row,new_col,gamestate);

                int move_info = (current_square == ENEMY_PIECE) ? CAPTURE_MOVE | KNIGHT_MOVE : KNIGHT_MOVE;
                if(current_square != FRIENDLY_PIECE && !is_king_threatened(row,col,new_row,new_col,board,0,gamestate))
                    add_move(row,col,new_row,new_col,moves,move_count,move_info);
            }
            new_row = row + 1 * dir1;
            new_col = col + 2 * dir2;
            if(new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8){
                int current_square = get_square_status(board,new_row,new_col,gamestate);

                int move_info = (current_square == ENEMY_PIECE) ? CAPTURE_MOVE | KNIGHT_MOVE : KNIGHT_MOVE;
                if(current_square != FRIENDLY_PIECE && !is_king_threatened(row,col,new_row,new_col,board,0,gamestate))
                    add_move(row,col,new_row,new_col,moves,move_count,move_info);
            }
        }
    }
}
// Computing horizontal, vertical and diagonal moves all follow the same pattern:
// 1. Loop the desired direction until we either hit our own piece or a enemy piece
// 2. Check that its not occupied by our own piece
// 3. Test if making the move opens our king
// Move_info variable is used to carry out if we move a rook so that we can handle castling related variables later on.
void compute_horizontal_moves(int row,int col,char** board,int* moves,int* move_count,int info,Gamestate* gamestate){
    for(int dir = -1; dir <= 1;dir += 2){
        int new_col = col + dir;
        int current_square = EMPTY_SQUARE;
        while(new_col < 8 && new_col >= 0 && current_square == EMPTY_SQUARE){
            current_square = get_square_status(board,row,new_col,gamestate);
            int move_info = (current_square == ENEMY_PIECE) ? CAPTURE_MOVE | info : info;
            if(current_square != FRIENDLY_PIECE  && !is_king_threatened(row,col,row,new_col,board,0,gamestate)){
                add_move(row,col,row,new_col,moves,move_count,move_info);
            }
            new_col += dir;
        }
    }
}
void compute_vertical_moves(int row,int col,char** board,int* moves,int* move_count, int info,Gamestate* gamestate){
    // Loop possible moves downwards and upwards
    for(int dir = -1; dir <= 1;dir += 2){
        int new_row = row + dir;
        int current_square = EMPTY_SQUARE;
        // While move is in bounds and current square is empty continue looping
        while(new_row < 8 && new_row >= 0 && current_square == EMPTY_SQUARE){
            current_square = get_square_status(board,new_row,col,gamestate);
            int move_info = (current_square == ENEMY_PIECE) ? CAPTURE_MOVE | info : info;
        if(current_square != FRIENDLY_PIECE && !is_king_threatened(row,col,new_row,col,board,0,gamestate)){
                add_move(row,col,new_row,col,moves,move_count,move_info);
            }
            new_row += dir;
        }
    }
}
void compute_diagonal_moves(int row,int col,char** board,int* moves,int* move_count,int info,Gamestate* gamestate){
    for(int dir1 = -1;dir1 <= 1; dir1 += 2){
        for(int dir2 = -1;dir2 <= 1;dir2 += 2){
            int current_square = EMPTY_SQUARE;
            int new_row = row + dir1;
            int new_col = col + dir2;
            while(new_row < 8 && new_row >= 0 && new_col < 8 && new_col >= 0 && current_square == EMPTY_SQUARE){

                current_square = get_square_status(board,new_row,new_col,gamestate);
                int move_info = (current_square == ENEMY_PIECE) ? CAPTURE_MOVE | info : info;
                if(current_square != FRIENDLY_PIECE && !is_king_threatened(row,col,new_row,new_col,board,0,gamestate)){
                    add_move(row,col,new_row,new_col,moves,move_count,move_info);
                }
                new_row += dir1;
                new_col += dir2;
            }
        }
    }
}
void compute_bishop_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate){
    compute_diagonal_moves(row,col,board,moves,move_count,BISHOP_MOVE,gamestate);
}
void compute_queen_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate){
    compute_vertical_moves(row,col,board,moves,move_count,QUEEN_MOVE,gamestate);
    compute_horizontal_moves(row,col,board,moves,move_count,QUEEN_MOVE,gamestate);
    compute_diagonal_moves(row,col,board,moves,move_count,QUEEN_MOVE,gamestate);
}
void compute_rook_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate){
    compute_horizontal_moves(row,col,board,moves,move_count,ROOK_MOVE,gamestate);
    compute_vertical_moves(row,col,board,moves,move_count,ROOK_MOVE,gamestate);
}
void compute_king_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate){
	int turn = gamestate->turn;
    for(int dir1 = -1; dir1 <= 1;dir1++){
        for(int dir2 = -1;dir2 <= 1;dir2++){
            int current_square = EMPTY_SQUARE;
            int new_row = row + dir1;
            int new_col = col + dir2;
            // If the move is in bounds and not the kings current square
            if(new_row < 8 && new_row >= 0 && new_col < 8 && new_col >= 0  && !(new_row == row && new_col == col)){
                current_square = get_square_status(board,new_row,new_col,gamestate);
                int move_info = (current_square == ENEMY_PIECE) ? CAPTURE_MOVE | KING_MOVE: KING_MOVE;
                // If the piece is not occupied by our own piece and moving to the square doesn't threaten king add the move
                if(current_square != FRIENDLY_PIECE && !is_king_threatened(row,col,new_row,new_col,board,KING_MOVE,gamestate)){
                add_move(row,col,new_row,new_col,moves,move_count,move_info);
                }
            }
        }
    }
    // If our king has not been moved we compute castling moves
    if(gamestate->king_moves & (1U<<turn))
        compute_castling(row,col,board,moves,move_count,gamestate);
}
void compute_castling(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate){
    int current_square = EMPTY_SQUARE;
    int castling_rights = 1;
    int move_info = KING_MOVE | CASTLING_MOVE;
    int turn = gamestate->turn;

    // Test if our left rook has been moved and that all squares between are empty
    if(gamestate->rook_moves & (1U<<turn)){
        for(int new_col = col - 1; new_col >= 1;new_col--){
            current_square = get_square_status(board,row,new_col,gamestate);
            if(current_square != EMPTY_SQUARE || is_king_threatened(row,col,row,new_col,board,KING_MOVE,gamestate))
                castling_rights = 0;
        }
        // If all squares meet the rules we add the mvoe
        if(castling_rights)
            add_move(row,col,row,col - 2,moves,move_count,move_info);
    }
    castling_rights = 1;
    // Test for other direction
    if(gamestate->rook_moves & 1U<<(turn + 2)){
    for(int new_col = col + 1; new_col <= 6;new_col++){
            current_square = get_square_status(board,row,new_col,gamestate);
            if(current_square != EMPTY_SQUARE || is_king_threatened(row,col,row,new_col,board,KING_MOVE,gamestate))
                castling_rights = 0;
        }
        if(castling_rights)
            add_move(row,col,row,col + 2,moves,move_count,move_info);
    }
}

