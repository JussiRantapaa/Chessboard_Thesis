
#ifndef GAMELOGIC_H_
#define GAMELOGIC_H_


#define CHAR_TO_INT 97
#define WHITE_PIECES 96
#define EMPTY_SQUARE ' '
#define ENEMY_PIECE 999
#define FRIENDLY_PIECE 888
#define WHITE 0
#define BLACK 1
#define ROW 55
#define COL 66

#define LEGAL_MOVE			(1U<<0)
#define ILLEGAL_MOVE		(1U<<1)

#define NORMAL_MOVE         (1U<<12)
#define KING_MOVE           (1U<<13)
#define CASTLING_MOVE       (1U<<14)
#define EN_PASSANT_MOVE     (1U<<15)
#define PAWN_DOUBLE_MOVE    (1U<<16)
#define CAPTURE_MOVE        (1U<<17)
#define ROOK_MOVE           (1U<<18)
#define PAWN_MOVE           (1U<<19)
#define KNIGHT_MOVE         (1U<<20)
#define BISHOP_MOVE         (1U<<21)
#define QUEEN_MOVE          (1U<<22)

#define PIECE_STARTING_POS_MASK 0xFFFF00000000FFFF
#define WHITE_STARTING_POS_MASK 0x000000000000FFFF

typedef struct gamestate{
    int kingpos[2];
    int turn;
    int pieces[2][16];
    int piece_count[2];
    int move_number;
    int move_history[300];
    int rook_moves;
    int king_moves;
    int game_over;
}Gamestate;

enum pieces{
    PAWN = 'p',
    ROOK = 'r',
    KNIGHT = 'n',
    BISHOP = 'b',
    QUEEN = 'q',
    KING ='k'
};

extern uint64_t BOARD_LAST_STATE;
void init_board(char **board,Gamestate* gamestate);
void display(char **board);
void display_winner(char** board,Gamestate* gamestate);
void ask_move(char **board,Gamestate* gamestate,uint32_t buffer[]);
int get_square_status(char** board,int row, int col,Gamestate* gamestate);
int get_random_index(int move_count);
void compute_moves(char** board,int* moves,int* move_count,Gamestate* gamestate);
int get_piece_index(int hash,Gamestate* gamestate,int piece_color);
void swap(int* a,int* b);

void handle_user_move(char **board,Gamestate* gamestate,uint32_t buffer[],int move_list[],int move_count);
void handle_bot_move(char **board,Gamestate* gamestate,uint32_t buffer[],int move_list[],int move_count);
void handle_game_ending(char **board,Gamestate* gamestate,uint32_t buffer[]);

uint64_t moves_per_square(int square,int* move_list,int move_count);

void compute_pawn_moves(int row,int col,char** board,int* moves, int* move_count,Gamestate* gamestate);
void compute_forward_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate);
void compute_capture_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate);
void compute_en_passant_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate);

void compute_knight_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate);
void compute_rook_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate);
void compute_bishop_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate);
void compute_queen_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate);
void compute_king_moves(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate);

void compute_diagonal_moves(int row,int col,char** board,int* moves,int* move_count,int info,Gamestate* gamestate);
void compute_horizontal_moves(int row,int col,char** board,int* moves,int* move_count,int info,Gamestate* gamestate);
void compute_vertical_moves(int row,int col,char** board,int* moves,int* move_count,int info,Gamestate* gamestate);

void compute_castling(int row,int col,char** board,int* moves,int* move_count,Gamestate* gamestate);
int test_rook_threat(int row,int col,int new_row,int new_col,char** board);
int test_bishop_threat(int row,int col,int new_row,int new_col,char** board);
int test_pawn_threat(int row,int col,int new_row,int new_col,char** board,Gamestate* gamestate);
int test_king_threat(int row,int col,int new_row,int new_col,char** board);
int test_queen_threat(int row,int col,int new_row,int new_col,char** board);
int test_knight_threat(int row,int col,int new_row,int new_col,char** board);


int is_king_threatened(int row,int col,int new_row,int new_col,char** board,int move_info,Gamestate* gamestate);
void add_move(int row,int col,int end_row,int end_col,int* moves,int* move_count,int move_info);
void make_move(char** board,int hash,Gamestate* gamestate);
void make_castling_move(char** board,int row,int col,int end_row,int end_col,Gamestate* gamestate);
int test_move(char **board,int row,int col,int end_row,int end_col,int* move_count,int* move_list);

int encode(int row,int col);
int decode(int hash,int choice);

int vertical_ray(int row,int col,int new_row,int new_col,char** board);
int horizontal_ray(int row,int col,int new_row,int new_col,char** board);
int diagonal_ray(int row,int col,int new_row,int new_col,char** board);


#endif /* GAMELOGIC_H_ */
