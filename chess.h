#ifndef CHESS_H_
#define CHESS_H_

// number of unique pieces per player on the board
#define CHESS_NUM_PIECES 6

#define BOARD_LENGTH 8
#define BOARD_HEIGHT 8

#define swith(num) ((num + 1) % 2)

#define BOARD_START(b_color) {\
	{{.pi = ROOK,  .c = swith(b_color)}, {.pi = KNIGHT, .c = swith(b_color)}, {.pi = BISHOP, .c = swith(b_color)}, {.pi = QUEEN, .c = swith(b_color)},\
	 {.pi = KING,  .c = swith(b_color)}, {.pi = BISHOP, .c = swith(b_color)}, {.pi = KNIGHT, .c = swith(b_color)}, {.pi = ROOK,  .c = swith(b_color)}},\
	{{.pi = PAWN,  .c = swith(b_color)}, {.pi = PAWN,   .c = swith(b_color)}, {.pi = PAWN,   .c = swith(b_color)}, {.pi = PAWN,  .c = swith(b_color)},\
	 {.pi = PAWN,  .c = swith(b_color)}, {.pi = PAWN,   .c = swith(b_color)}, {.pi = PAWN,   .c = swith(b_color)}, {.pi = PAWN,  .c = swith(b_color)}},\
	\
	{{.pi = BLANK, .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK, .c = WHITE},\
	 {.pi = BLANK, .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK, .c = WHITE}},\
	{{.pi = BLANK, .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK, .c = WHITE},\
	 {.pi = BLANK, .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK, .c = WHITE}},\
	{{.pi = BLANK, .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK, .c = WHITE},\
	 {.pi = BLANK, .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK, .c = WHITE}},\
	{{.pi = BLANK, .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK, .c = WHITE},\
	 {.pi = BLANK, .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK,  .c = WHITE}, {.pi = BLANK, .c = WHITE}},\
	\
	{{.pi = PAWN,  .c = b_color}, {.pi = PAWN,   .c = b_color}, {.pi = PAWN,   .c = b_color}, {.pi = PAWN,  .c = b_color},\
	 {.pi = PAWN,  .c = b_color}, {.pi = PAWN,   .c = b_color}, {.pi = PAWN,   .c = b_color}, {.pi = PAWN,  .c = b_color}},\
	{{.pi = ROOK,  .c = b_color}, {.pi = KNIGHT, .c = b_color}, {.pi = BISHOP, .c = b_color}, {.pi = QUEEN, .c = b_color},\
	 {.pi = KING,  .c = b_color}, {.pi = BISHOP, .c = b_color}, {.pi = KNIGHT, .c = b_color}, {.pi = ROOK,  .c = b_color}}\
}

typedef enum {
	F_PAWN = -2, BLANK = -1,
	PAWN = 0, ROOK = 1, KNIGHT = 2, BISHOP = 3, QUEEN = 4, KING = 5
} chess_p;

typedef enum {
	NOCOLOR = -1,
	WHITE = 0, BLACK = 1
} color;

typedef struct {
	chess_p pi;
	color c;
} chess_piece;

typedef chess_piece board[BOARD_LENGTH][BOARD_HEIGHT];

typedef enum {
	C_QUEEN = -1, C_KING = -2,
	B_CASTLE_QUEEN = 0x01,
	B_CASTLE_KING = 0x02,
	W_CASTLE_QUEEN = 0x04,
	W_CASTLE_KING = 0x08
} castle_state;

typedef struct {
	board b;
	color turn, check;
	castle_state castle;
} chess_t;

//void chess_init(chess_t*);
void reset(chess_t*);
char move(chess_t*, char *);
char *print_color(color);

//extern color chess_turn;

#endif /* CHESS_H_ */
