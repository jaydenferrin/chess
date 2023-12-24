#include "chess.h"
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#define CHESS_ERR -1
#define CHESS_NORMAL 0
#define CHESS_CHECK 1
#define CHESS_MATE 2
#define CHESS_STALE 3

typedef enum {
	MOVE_CHECK   = 0x1,
	MOVE_MATE    = 0x2,
	MOVE_PROMOTE = 0x4,
	MOVE_CASTLE  = 0x8,
	MOVE_CAPTURE = 0x16
} move_flags;

typedef struct {
	int x, y, tx, ty;
	chess_p piece;
	move_flags flags;
} move_t;

//typedef void (*intop)(int*, int);

void print_piece(chess_piece);
void rm_phantoms(board, int, int);

//color chess_turn = WHITE;
bool check[] = {false, false};

static int abs(int in) {
	return (in < 0) ? -in : in;
}

static bool out_of_bounds(int x, int y) {
	return x < 0 || y < 0 || x >= BOARD_LENGTH || y >= BOARD_HEIGHT;
}

// returns a bool if movement took a piece
static bool _move(board b, int x, int y, int tx, int ty) {
	int behind = ((y < ty) ? -1 : 1);
	chess_piece piece = b[y][x], prev = b[ty][tx];
	bool ret = prev.pi != BLANK;
//	if (piece.pi == BLANK || (b[ty][tx].c == piece.c && b[ty][tx].pi != BLANK))
//		return false;
	b[ty][tx] = piece;
	b[y][x].pi = BLANK;
	b[y][x].c = WHITE;
	if (piece.pi == PAWN) {
		if (abs(y - ty) == 2) {
			// creating a phantom pawn when moving forward 2
			b[ty + behind][tx].pi = F_PAWN;
			b[ty + behind][tx].c = piece.c;
			goto rm;
		}
		if (prev.pi == F_PAWN) {
			b[ty + behind][tx].pi = BLANK;
			b[ty + behind][tx].c = WHITE;
		}
//		if (abs(y - ty) == 1 && (b[y + behind][x].pi == F_PAWN && b[y + behind][x].c == piece.c)) {
//			b[y + behind][x].pi = BLANK;
//			b[y + behind][x].c = WHITE;
//		}
//		if (prev.pi == PAWN && b[ty - behind][tx].pi == F_PAWN) {
//			b[ty - behind][tx].pi = BLANK;
//			b[ty - behind][tx].c = WHITE;
//			return;
//		}
	}
rm:
	rm_phantoms(b, tx, ty + behind);
	return ret;

//	printf("from %d, %d: ", x, y);
//	print_piece(b[y][x]);
//	printf("to %d, %d: ", tx, ty);
//	print_piece(b[ty][tx]);
//	printf("\n");

//	return true;
}

//static void nop(int *n, int no) {}
//
//static void inc(int *x, int op) {
//	*x += op;
//}

// returns true if there are no obstructions between (fromx, fromy) and (tox, toy) 
// in board b, changing x and y by their respective functions;
static bool check_line(const board b, int fromx, int fromy, int tox, int toy) {
	//xop(&fromx, (fromx < tox) ? 1 : -1);
	fromx += (fromx == tox) ? 0 : ((fromx < tox) ? 1 : -1);
	fromy += (fromy == toy) ? 0 : ((fromy < toy) ? 1 : -1);
	//yop(&fromy, (fromy < toy) ? 1 : -1);
	if (fromx == tox && fromy == toy) 
		return true;
	if (b[fromy][fromx].pi != BLANK)
		return false;
	return check_line(b, fromx, fromy, tox, toy);
}

static bool can_move(board b, int x, int y, int tx, int ty) {
//#define pawn_print(msg) fprintf(stderr, "ERROR: pawn at %d, %d, moving to %d, %d %s\n", x, y, tx, ty, msg)
	if (out_of_bounds(tx, ty)) {
		//fprintf(stderr, "ERROR: out of bounds\n");
		return false;
	}
	if (tx == x && ty == y) {
		//fprintf(stderr, "ERROR: didn't move\n");
		return false;
	}
	chess_piece p = b[y][x];
	if (p.c == b[ty][tx].c && (b[ty][tx].pi != BLANK && b[ty][tx].pi != F_PAWN)) {
		//fprintf(stderr, "ERROR: friendly fire\n");
		return false;
	}
	if (p.pi == BLANK)
		return false;

	int diffx = abs(tx - x);
	int diffy = abs(ty - y);
//	intop xo, yo;
	switch (p.pi) {
		case PAWN:
			if ((y <= ty && p.c == WHITE) || (ty <= y && p.c == BLACK)) {
//				pawn_print("moving backwards");
				return false;
			}
			//int dy = ty - y;
			if (diffx > 1) {
//				pawn_print("too far x dir");
				return false;
			}
			if (diffy > 2) {
//				pawn_print("too far forwards");
				return false;
			}
			int origpos = 1 + swith(p.c) * 5;
			if (diffy == 2 && (y != origpos || diffx > 0)) {
//				pawn_print("not in origpos");
//				printf("%d\n", origpos);
				return false;
			}
			if (diffx == 0 && (b[ty][tx].pi != BLANK)) {
//				pawn_print("something in the way");
				return false;
			}
			if (diffx == 0 && diffy == 2 && !check_line(b, x, y, tx, ty)) {
//				pawn_print("something in the way 2");
				return false;
			}
			if (diffx == 1 && (b[ty][tx].pi == BLANK && !(b[ty][tx].pi == F_PAWN && b[ty][tx].c != p.c))) {
//				pawn_print("nothing to take");
				return false;
			}
//			pawn_print("has made it");
			return true;
			break;
		case ROOK:
			if (diffy > 0 && diffx > 0)
				return false;
			//intop xo, yo;
//			xo = (diffx > 0) ? inc : nop;
//			yo = (diffy > 0) ? inc : nop;
			return check_line(b, x, y, tx, ty);
			break;
		case KNIGHT:
			if (diffx <= 1 && diffy <= 1) 	// inner square (king's moves)
				return false;
			if (diffx > 2 || diffy > 2) 	// outer square
				return false;
			if (diffx == diffy) 		// diagonals (bishop's moves)
				return false;
			return diffy > 0 && diffx > 0;	// perpendiculars (rook's moves)
			break;
		case BISHOP:
			if (diffy != diffx)
				return false;
			return check_line(b, x, y, tx, ty);
			break;
		case QUEEN:
			if (diffy != diffx && (diffy > 0 && diffx > 0))
				return false;
			//intop xo, yo;
//			xo = (diffx > 0) ? inc : nop;
//			yo = (diffy > 0) ? inc : nop;
			return check_line(b, x, y, tx, ty);
			break;
		case KING:
			return diffx <= 1 && diffy <= 1;
			break;
		default:
			return false;
	}

}

static chess_p parse_piece(char in) {
	switch (in) {
		case 'Q':
			return QUEEN;
		case 'N':
			return KNIGHT;
		case 'B':
			return BISHOP;
		case 'R':
			return ROOK;
		case 'K':
			return KING;
		default:
			fprintf(stderr, "character %c does not represent a piece\n", in);
			return  BLANK;
	}
}

char *print_p(chess_p p) {
	switch (p) {
		case PAWN:
			return "pawn";
		case ROOK:
			return "rook";
		case KNIGHT:
			return "knight";
		case BISHOP:
			return "bishop";
		case QUEEN:
			return "queen";
		case KING:
			return "king";
		default:
			return "blank";
	}
}

void print_piece(chess_piece piece) {
#define fff_print(color, name) printf("%s %s", (color == WHITE) ? "white" : "black", name)
	switch (piece.pi) {
		case PAWN:
			fff_print(piece.c, "pawn");
			break;
		case ROOK:
			fff_print(piece.c, "rook");
			break;
		case KNIGHT:
			fff_print(piece.c, "knight");
			break;
		case BISHOP:
			fff_print(piece.c, "bishop");
			break;
		case QUEEN:
			fff_print(piece.c, "queen");
			break;
		case KING:
			fff_print(piece.c, "king");
			break;
		default:
			printf("blank");
			break;
	}
#undef fff_print
}

bool incheck(board b, int x, int y) {
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_LENGTH; j++) {
			if (can_move(b, j, i, x, y))
				return true;
		}
	}
	return false;
}

bool incheck_no_buf(board b, int kx, int ky, int fromx, int fromy, int tox, int toy) {
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_LENGTH; j++) {
			int x = j, y = i;
			if (x == fromx && y == fromy) {
				x = tox;
				y = toy;
			}
			if (can_move(b, x, y, kx, ky))
				return true;
		}
	}
	return false;
}

void rm_phantoms(board b, int x, int y) {
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_LENGTH; j++) {
			if (j == x && i == y) continue;
			if (b[i][j].pi == F_PAWN) {
				b[i][j].pi = BLANK;
				b[i][j].c = WHITE;
			}
		}
	}	

}

bool legal_move_exists(board *b, color turn, int kx, int ky) {
	board tmp;
	int kingx = kx, kingy = ky;
	for (int y = 0; y < BOARD_HEIGHT; y++) {
		for (int x = 0; x < BOARD_LENGTH; x++) {
			if ((*b)[y][x].pi == BLANK || (*b)[y][x].c != turn)
				continue;
			for (int ty = 0; ty < BOARD_HEIGHT; ty++) {
				for (int tx = 0; tx < BOARD_LENGTH; tx++) {
					if ((*b)[ty][tx].c == turn && (*b)[ty][tx].pi != BLANK)
						continue;
					if (!can_move(*b, x, y, tx, ty))
						continue;
					if (kx == x && ky == y) {
						kingx = tx;
						kingy = ty;
//						if (can_move(*b, x, y, tx, ty) && !incheck_no_buf(*b, tx, ty, x, y, tx, ty))
//							return true;
//						continue;
					}
					memcpy(*tmp, **b, sizeof *b);
					_move(tmp, x, y, tx, ty);
					if (!incheck(tmp, kingx, kingy))
						return true;
				}
			}
		}
	}
	return false;
}

static bool disambiguation(char *disambig, char *dest, int *x, int *y) {
	if (*disambig == 'x' && disambig + 1 == dest)
		return true;
	*y = -1;
	*x = *disambig - 'a';
	if (*x < 0) {
		*x = -1;
		*y = BOARD_HEIGHT - (*disambig - '0');
	}
	if (((*x >= BOARD_LENGTH || *x < 0) && -1 == *y) || ((*y >= BOARD_HEIGHT || *y < 0) && -1 == *x))
		return false;
	if (++disambig == dest)
		return true;
	if (*disambig == 'x' && disambig + 1 == dest)
		return true;
	// the next character is part of the disambiguation
	// guaranteed to be y
	if (*y != -1)
		return false;
	*y = BOARD_HEIGHT - (*disambig - '0');
	if (*y >= BOARD_HEIGHT || *y < 0)
		return false;
	if (++disambig == dest)
		return true;
	if (*disambig == 'x' && disambig + 1 == dest)
		return true;
	return false;
}

static bool parse_flag(char flag, move_t *move) {
	switch (flag) {
		case '+':
			move->flags |= MOVE_CHECK;
			return true;
		case '#':
			move->flags |= MOVE_MATE;
			return true;
		default:
			return false;
	}
}

/*
 * This function parses a movement string into a move type,
 * but makes no checks if the move is possible
 */
static bool parse_movement(char *notation, color turn, move_t *move, char *promote) {
	size_t length = strlen(notation);
	move->flags = 0;
	move->x = -1;
	move->y = -1;
	if (parse_flag(notation[length - 1], move))
		length--;
	if (*notation == '0' || *notation == 'o' || *notation == 'O') {
		char cch[3] = " -";
		cch[0] = *notation;
		//castb = true;
		move->piece = KING;
		int cnum = 1;
		char *not = notation;
		while (strncmp(not += 2, cch, 2) == 0 && ++cnum < 2);
		if ((cnum * 2) + 1 != length || *not != *cch)
			return false;
		
		// cnum is 1 if kingside, 2 if queenside
		move->tx = (cnum == 2) ? 2 : BOARD_LENGTH - 2;
		move->ty = (turn == BLACK) ? 0 : BOARD_HEIGHT - 1;
		move->x = 4;
		move->y = move->ty;
		move->flags |= MOVE_CASTLE;
		return true;
	}

	char *dest = notation + (length - 2);
	char *disambig = notation;
	promote = NULL;

	if (isupper(dest[1])) {	// promoting a pawn
		promote = dest + 1;
		dest--;
		move->flags |= MOVE_PROMOTE;
	}

	if (isupper(*notation)) {
		move->piece = parse_piece(*notation);
		disambig++;
	}

	move->ty = 8 - (dest[1] - '0');
	move->tx = *dest - 'a';
	if (move->tx >= 8 || move->tx < 0 || move->ty >= 8 || move->ty < 0)
		return false;

	if (disambig != dest && !disambiguation(disambig, dest, &(move->x), &(move->y)))
		return false;
	return true;
}

//char *print_p(chess_p piece) {
//	switch (piece) {
//		case PAWN:
//			return "pawn";
//		case ROOK:
//			return "rook";
//		case KNIGHT:
//			return "knight";
//		case BISHOP:
//			return "bishop";
//		case QUEEN:
//			return "queen";
//		case KING:
//			return "king";
//		default:
//			return "blank";
//	}
//}

static bool find_x_y(chess_t *chess_board, move_t *move) {
	if (move->x > -1 && move->y > -1)
		return true;
	int matches = 0;
	for (int i = 0; i < BOARD_HEIGHT; ++i) {
		for (int j = 0; j < BOARD_LENGTH; ++j) {
			// check if this piece matches the piece we are trying to move
			if (!((chess_board->b)[i][j].pi == move->piece &&
			      (chess_board->b)[i][j].c == chess_board->turn))
				continue;
			// check if this piece can move to the position we are trying to move to 
			if (!can_move(chess_board->b, j, i, move->tx, move->ty))
				continue;
			// check if this piece matches given position constraints, if any were given
			if ((move->x > -1 && j != move->x) || (move->y > -1 && i != move->y))
				continue;
			matches++;
			// assume this is the right piece
			move->x = j;
			move->y = i;
		}
	}
	if (matches != 1)
		return false;
	return true;
}

static bool promotion(board b, move_t *move, char* promote) {
	if (NULL == promote)
		return false;
	chess_p prom = parse_piece(*promote);
	if (BLANK == prom)
		return false;
	b[move->ty][move->tx].pi = prom;
	return true;
}

static bool test_move(chess_t *chess_board, move_t *move, board buf, char *promote, castle_state *state) {
	castle_state col = (chess_board->turn == WHITE) ? W_CASTLE_QUEEN : B_CASTLE_QUEEN;
	if (move->tx != 0)
		col *= 2;
	// one can castle if and only if
	// 1: neither the king nor the rook castling has been moved
	// 2: the line is clear between them
	// 3: the king is not in check
	if ((move->flags & ~MOVE_CASTLE) &&
			((chess_board->castle & col) == 0 ||
			!check_line(chess_board->b, move->x, move->y, move->tx, move->ty) || 
			NOCOLOR != chess_board->check))
		return false;
	if ((move->flags & ~MOVE_PROMOTE) && move->piece != PAWN &&
			move->ty != (BOARD_HEIGHT - 1) * swith(chess_board->turn))
		return false;
	memcpy(*buf, *chess_board->b, sizeof chess_board->b);
	// make the move on the temporary board
	bool taken = _move(buf, move->x, move->y, move->tx, move->ty);
	if ((move->flags & ~MOVE_CAPTURE) && !taken)
		return false;
	int kcopy[2];
	memcpy(kcopy, chess_board->kpos[chess_board->turn], sizeof kcopy);
	castle_state cstate = 0;
	switch (move->piece) {
		case ROOK:
			castle_state tstate = (chess_board->turn == WHITE) ? W_CASTLE_QUEEN : B_CASTLE_QUEEN;
			tstate *= (move->x == 0) ? 1 : 2;
			cstate |= tstate;
			break;
		case KING:
			// update king's position 
			kcopy[0] = move->tx;
			kcopy[1] = move->ty;
			cstate = (chess_board->turn == WHITE) ? W_CASTLE_QUEEN : B_CASTLE_QUEEN;
			cstate |= cstate * 2;
			break;
		default:
			break;
	}
	if (incheck(buf, kcopy[0], kcopy[1]))
		return false;
	if (move->flags & ~MOVE_CASTLE) {
		// we are castling, move the rook
		castle_state tstate = (chess_board->turn == WHITE) ? W_CASTLE_QUEEN : B_CASTLE_QUEEN;
		int x = (tstate == col) ? 0 : BOARD_LENGTH - 1;
		int tx = (tstate == col) ? 3 : BOARD_LENGTH - 3;
		_move(buf, x, move->y, tx, move->ty);
	}
	*state = cstate;
	if (move->flags & ~MOVE_PROMOTE)
		return promotion(buf, move, promote);
	return true;
}

char move(chess_t *chess_board, char *notation) {
	move_t move;
	char *promote = NULL;
	if (!parse_movement(notation, chess_board->turn, &move, promote)) {
		fprintf(stderr, "notation syntax error: %s\n", notation);
		return CHESS_ERR;
	}
	if (!find_x_y(chess_board, &move)) {
		fprintf(stderr, "ambiguous input or no piece can achieve %s\n", notation);
		return CHESS_ERR;
	}
	board tmp_b;
	castle_state cstate;
	if (!test_move(chess_board, &move, tmp_b, promote, &cstate)) {
		fprintf(stderr, "unable to make %s\n", notation);
		return CHESS_ERR;
	}
	memcpy(*chess_board->b, *tmp_b, sizeof chess_board->b);
	chess_board->castle &= ~cstate;
	chess_board->turn = swith(chess_board->turn);
	chess_board->check = NOCOLOR;
	if (incheck(chess_board->b, chess_board->kpos[chess_board->turn][0], 
				chess_board->kpos[chess_board->turn][1])) {
		chess_board->check = chess_board->turn;
	}
	if (legal_move_exists(&chess_board->b, 
				chess_board->turn, 
				chess_board->kpos[chess_board->turn][0], 
				chess_board->kpos[chess_board->turn][1])) {
		return (NOCOLOR == chess_board->check) ? CHESS_NORMAL : CHESS_CHECK;
	}
	// no legal move exists
	return (NOCOLOR == chess_board->check) ? CHESS_STALE : CHESS_MATE;
}

// returns -1 if error,
// 0 if normal
// 1 if in check
// 2 if checkmate
char unmove(chess_t *chess_board, char *notation) {
	// kpos stores each king's position x, y indexed by its color
	//static color last_turn = BLACK;
//	static int kpos[][2] = {
//		{4, 7},	// white king x, y
//		{4, 0}	// black king x, y
//	};

	//static castle_state castle = B_CASTLE_KING | B_CASTLE_QUEEN | W_CASTLE_KING | W_CASTLE_QUEEN;

//	static color cheek = -1;	// who's in check

	int length = strlen(notation);
	char *dest, *disambig, *promote = NULL;
	chess_piece piece = {.pi = PAWN, .c = chess_board->turn};
	int tx, ty, x, y;
	unsigned int matches = 0;
	int *k = chess_board->kpos[piece.c];
	castle_state col = 0, cstate = 0;

	char cch[3] = " -";
	if (*notation == '0' || *notation == 'o' || *notation == 'O') {
		cch[0] = *notation;
		//castb = true;
		piece.pi = KING;
		int cnum = 1;
		char *not = notation;
		while (strncmp(not += 2, cch, 2) == 0 && ++cnum < 2);
		if ((cnum * 2) + 1 != length || *not != *cch) {
			fprintf(stderr, "invalid input %s\n", notation);
			return CHESS_ERR;
		}
		//fprintf(stderr, "%d reps\n", cnum);

		col = (piece.c == WHITE) ? W_CASTLE_QUEEN : B_CASTLE_QUEEN;
		// cnum is 1 if kingside, 2 if queenside
		col *= (cnum % 2) + 1;

		tx = (cnum == 2) ? 2 : BOARD_LENGTH - 2;
		ty = (piece.c == BLACK) ? 0 : BOARD_HEIGHT - 1;
		x = 4;
		y = ty;
		matches = 1;

		// one can castle if and only if
		// 1: neither the king nor the rook castling has been moved
		// 2: the line is clear between them
		// 3: the king is not in check
		if ((chess_board->castle & col) == 0 || !check_line(chess_board->b, x, y, tx, ty) || NOCOLOR != chess_board->check) {
			fprintf(stderr, "unable to castle\n");
			return CHESS_ERR;
		}
		goto castling;
	}
	//length = strlen(notation);
	dest = notation + (length - 2);
	disambig = notation;
	promote = NULL;
	if (isupper(dest[1])) {
		promote = dest + 1;
		dest--;	// promoting a pawn
	}

	//piece = {.pi = PAWN, .c = chess_board->turn};
	//piece.c = chess_board->turn;
	if (isupper(*notation)) {
		piece.pi = parse_piece(*notation);
		disambig++;
	}
//	if (*disambig == 'x') disambig++;

//	print_piece(piece);
//	printf("\n");

	tx = *dest - 'a';
	ty = 8 - (dest[1] - '0');
//	printf("moving to %d, %d\n", tx, ty);
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_LENGTH; j++) {
			//if (can_move(chess_board->b, j, i, k[0], k[1]) && !check[chess_board->turn])
			//	check[chess_board->turn] = true;
			if (!((chess_board->b)[i][j].pi == piece.pi && (chess_board->b)[i][j].c == piece.c))
				continue;
//			printf("found ");
//			print_piece(piece);
//			printf(" at %d, %d\n", j, i);
			if (can_move(chess_board->b, j, i, tx, ty)) {
				matches++;
				if (matches > 1 && disambig != dest) {
					char file = *disambig - 'a';
					char rank = 8 - (*disambig - '0');
					bool stop = false;
search:
					if (x == file || y == rank) {
						matches = 1;
						goto found;
					}
					if (j == file || i == rank) {
						matches = 1;
						x = j;
						y = i;
						goto found;
					}
					if (!stop) {
						rank = 8 - (disambig[1] - '0');
						stop = true;
						goto search;
					}
				}
				x = j;
				y = i;
			}
		}
	}
	goto found;
found:	
	if (matches == 0) {
		fprintf(stderr, "invalid input %s\n", notation);
		return CHESS_ERR;
	}
	if (matches > 1) {
		fprintf(stderr, "input %s too ambiguous\n", notation);
		return CHESS_ERR;
	}

	board tmp;
	//bool castb = false;
castling:
	memcpy(*tmp, *chess_board->b, sizeof chess_board->b);

	// checking if the current player's king is in check
	// or if this movement will put them in check
	bool taken = _move(tmp, x, y, tx, ty);
	int kcopy[2];
	memcpy(kcopy, k, sizeof kcopy);
	if (piece.pi == ROOK) {
		castle_state tstate = (piece.c == WHITE) ? W_CASTLE_QUEEN : B_CASTLE_QUEEN;
		tstate *= (x == 0) ? 1 : 2;
		cstate |= tstate;
	} else if (piece.pi == KING) {
		// update king's position 
		k[0] = tx;
		k[1] = ty;
		cstate = (piece.c == WHITE) ? W_CASTLE_QUEEN : B_CASTLE_QUEEN;
		cstate |= cstate * 2;
	}
	if (incheck(tmp, k[0], k[1])) {
		fprintf(stderr, "cannot perform %s king in check!\n", notation);
		memcpy(k, kcopy, sizeof kcopy);
		return CHESS_ERR;
	}
	if (0 != col) {
		// we are castling, move the rook
		castle_state tstate = (piece.c == WHITE) ? W_CASTLE_QUEEN : B_CASTLE_QUEEN;
		x = (tstate == col) ? 0 : BOARD_LENGTH - 1;
		tx = (tstate == col) ? 3 : BOARD_LENGTH - 3;
		_move(tmp, x, y, tx, ty);
	}
	// tried to take a piece in notation, but no piece was taken
	if (memchr(notation, 'x', length) && !taken) {
		fprintf(stderr, "no piece taken\n");
		return CHESS_ERR;
	}
	// pawn promotion
	if (NULL != promote) {
		if (piece.pi != PAWN) {
			fprintf(stderr, "cannot promote a %s\n", print_p(piece.pi));
			return CHESS_ERR;
		}
		chess_p prom;
		if ((prom = parse_piece(*promote)) == BLANK) {
			fprintf(stderr, "%c does not refer to a piece\n", *promote);
			return CHESS_ERR;
		}
		if (ty != ((piece.c == BLACK) ? BOARD_HEIGHT - 1 : 0)) {//(BOARD_HEIGHT - 1) - (piece.c * (BOARD_HEIGHT - 1))) {
			fprintf(stderr, "cannot promote pawn at %c%c\n", dest[0], dest[1]);
			return CHESS_ERR;
		}
		tmp[ty][tx].pi = prom;
	}

	// if we get here we are not in check and the movement passed all other tests
	memcpy(*chess_board->b, *tmp, sizeof chess_board->b);
	chess_board->castle &= ~cstate;
	//rm_phantoms(chess_board->b);
	//_move(chess_board->b, x, y, tx, ty);
	//last_turn = chess_board->turn;
	chess_board->turn = swith(chess_board->turn);
	if (incheck(chess_board->b, chess_board->kpos[chess_board->turn][0], 
				chess_board->kpos[chess_board->turn][1])) {
		chess_board->check = chess_board->turn;
	}
	if (!legal_move_exists(&chess_board->b, chess_board->turn, chess_board->kpos[chess_board->turn][0], 
				chess_board->kpos[chess_board->turn][1])) {
		if (NOCOLOR != chess_board->check) {
			chess_board->check = NOCOLOR;
			fprintf(stderr, "%s wins by checkmate!\n", print_color(swith(chess_board->check)));
			return CHESS_MATE;
		}
		fprintf(stderr, "game ends by stalemate!\n");
		return CHESS_STALE;
	}
	if (NOCOLOR != chess_board->check) {
		fprintf(stderr, "%s in check!\n", print_color(chess_board->check));
		chess_board->check = NOCOLOR;
		return CHESS_CHECK;
	}
	chess_board->check = NOCOLOR;
	return CHESS_NORMAL;
}

void reset(chess_t *chess_board) {
	board tmp = BOARD_START(WHITE);
	memcpy(*(chess_board->b), *tmp, sizeof chess_board->b);
	chess_board->turn = WHITE;
	chess_board->check = NOCOLOR;
	chess_board->castle = B_CASTLE_KING | B_CASTLE_QUEEN | W_CASTLE_KING | W_CASTLE_QUEEN;
	chess_board->kpos[0][0] = 4;
	chess_board->kpos[0][1] = 7;
	chess_board->kpos[1][0] = 4;
	chess_board->kpos[1][1] = 0;

}

//void chess_init(chess_t *chess_board) {
//	board tmp = BOARD_START(WHITE);
//	memcpy(*(chess_board->b), *tmp, sizeof chess_board->);
//	chess_board->turn = WHITE;
//	chess_board->castle = B_CASTLE_KING | B_CASTLE_QUEEN | W_CASTLE_KING | W_CASTLE_QUEEN;
//}

//board chess_get_board(chess_t *chess_board) {
//	return chess_board->b;
//}
//
//color turn(chess_t *chess_board) {
//	return chess_board->turn;
//}

char *print_color(color c) {
	switch (c) {
		case WHITE:
			return "white";
		case BLACK:
			return "black";
		default:
			return NULL;
	}
}
