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
	int kpos[2][2];
	color turn, check;
	castle_state castle;
	unsigned int moves;
	unsigned int h_len, h_end;
	char *history;
} chess_t;

typedef enum {
	CHESS_ERR = -16,
	CHESS_ERR_PROMISE = -5,
	CHESS_ERR_ILLEGAL = -4,
	CHESS_ERR_NOAVAIL = -3,
	CHESS_ERR_AMBIG = -2,
	CHESS_ERR_PARSE = -1,
	CHESS_NORMAL = 0,
	CHESS_CHECK = 1,
	CHESS_MATE = 2,
	CHESS_STALE = 3,
	CHESS_END = 2,
} chess_return;

//void chess_init(chess_t*);
void reset(chess_t*);
chess_return move(chess_t*, char *);
char *print_color(color);
void cleanup (chess_t*);

//extern color chess_turn;

#endif /* CHESS_H_ */
