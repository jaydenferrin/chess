#include "chess.h"
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
	MOVE_CAPTURE = 0x10
} move_flags;

typedef struct {
	int x, y, tx, ty;
	chess_p piece;
	move_flags flags;
} move_t;

static void print_piece(chess_piece);
static void rm_phantoms(board, int, int);

//static int abs(int in) {
//	return (in < 0) ? -in : in;
//}

static bool out_of_bounds(int x, int y) {
	return x < 0 || y < 0 || x >= BOARD_LENGTH || y >= BOARD_HEIGHT;
}

// returns a bool if movement took a piece
static bool _move(board b, int x, int y, int tx, int ty) {
	int behind = ((y < ty) ? -1 : 1);
	chess_piece piece = b[y][x], prev = b[ty][tx];
	bool ret = prev.pi != BLANK;
	b[ty][tx] = piece;
	b[y][x].pi = BLANK;
	b[y][x].c = WHITE;
	if (piece.pi == PAWN) {
		if (abs(y - ty) == 2) {
			// creating a phantom pawn when moving forward 2
			b[ty + behind][tx].pi = F_PAWN;
			b[ty + behind][tx].c = piece.c;
		} else if (prev.pi == F_PAWN) {
			b[ty + behind][tx].pi = BLANK;
			b[ty + behind][tx].c = WHITE;
		}
	}
	rm_phantoms(b, tx, ty + behind);
	return ret;
}

// returns true if there are no obstructions between (fromx, fromy) and (tox, toy) 
// in board b, changing x and y by their respective functions;
static bool check_line(const board b, int fromx, int fromy, int tox, int toy) {
	fromx += (fromx == tox) ? 0 : ((fromx < tox) ? 1 : -1);
	fromy += (fromy == toy) ? 0 : ((fromy < toy) ? 1 : -1);
	if (fromx == tox && fromy == toy) 
		return true;
	if (b[fromy][fromx].pi != BLANK)
		return false;
	return check_line(b, fromx, fromy, tox, toy);
}

static bool can_move_minimal(board b, int x, int y, int tx, int ty) {
	// check if this movement is out of bounds
	if (out_of_bounds(tx, ty)) {
		return false;
	}
	// check if the target is the same as the starting position
	if (tx == x && ty == y) {
		return false;
	}
	chess_piece p = b[y][x];
	// check for friendly fire
	if (p.c == b[ty][tx].c && (b[ty][tx].pi != BLANK && b[ty][tx].pi != F_PAWN)) {
		return false;
	}
	if (p.pi == BLANK)
		return false;
	return true;
}

static bool can_move(board b, int x, int y, int tx, int ty) {
	if (!can_move_minimal(b, x, y, tx, ty))
		return false;

	chess_piece p = b[y][x];

	int diffx = abs(tx - x);
	int diffy = abs(ty - y);
	switch (p.pi) {
		case PAWN:
			if ((y <= ty && p.c == WHITE) || (ty <= y && p.c == BLACK)) {
				return false;
			}
			if (diffx > 1) {
				return false;
			}
			if (diffy > 2) {
				return false;
			}
			int origpos = 1 + swith(p.c) * 5;
			if (diffy == 2 && (y != origpos || diffx > 0)) {
				return false;
			}
			if (diffx == 0 && (b[ty][tx].pi != BLANK)) {
				return false;
			}
			if (diffx == 0 && diffy == 2 && !check_line(b, x, y, tx, ty)) {
				return false;
			}
			if (diffx == 1 && (b[ty][tx].pi == BLANK && !(b[ty][tx].pi == F_PAWN && b[ty][tx].c != p.c))) {
				return false;
			}
			return true;
			break;
		case ROOK:
			if (diffy > 0 && diffx > 0)
				return false;
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

static char *unparse_piece(chess_p p) {
	switch (p) {
		case PAWN:
			return "";
		case ROOK:
			return "R";
		case KNIGHT:
			return "N";
		case BISHOP:
			return "B";
		case QUEEN:
			return "Q";
		case KING:
			return "K";
		default:
			return "";
	}
}

static char *print_p(chess_p p) {
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

static void print_piece(chess_piece piece) {
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

static bool incheck(board b, int kx, int ky) {
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_LENGTH; j++) {
			if (can_move(b, j, i, kx, ky))
				return true;
		}
	}
	return false;
}

static void rm_phantoms(board b, int x, int y) {
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

/*
 * returns true if it can keep going, false otherwise
 */
static bool pawn_movement(board b, int sign, int x, int y, int *tx, int *ty, int *iterations) {
	switch (*iterations) {
		case -1:
			return false;
		case 0:
			*tx = x;
			*ty = y + sign;
			break;
		case 1:
			*tx = x - 1;
			*ty = y + sign;
			break;
		case 2:
			*tx = x + 1;
			*ty = y + sign;
			break;
		case 3:
			if (x != (BOARD_HEIGHT + sign - 1) % 7)
				return false;
			*tx = x;
			*ty = y + 2 * sign;
			break;
		default:
			*iterations = -1;
			return false;
	}
	*iterations = *iterations + 1;
	return can_move_minimal(b, x, y, *tx, *ty);
}

static bool diag_movement(board b, int dir, int x, int y, int *tx, int *ty, int *iterations) {
	if (dir > 3 || dir < 0 || *iterations < 0) {
		*iterations = -1;
		return false;
	}
	int xdir = (dir / 2) * 2 - 1;
	int ydir = (((dir + 1) % 4) / 2) * 2 - 1;
	*tx = x + (*iterations + 1) * xdir;
	*ty = y + (*iterations + 1) * ydir;
	if (!can_move_minimal(b, x, y, *tx, *ty)) {
		return false;
	}
	*iterations = *iterations + 1;
	return true;
}

static bool cross_movement(board b, int dir, int x, int y, int *tx, int *ty, int *iterations) {
	if (dir > 3 || dir < 0 || *iterations < 0) {
		*iterations = -1;
		return false;
	}
	int xdir, ydir;
	switch (dir) {
		case 0:	// up
			xdir = 0;
			ydir = 1;
			break;
		case 1:	// right
			xdir = 1;
			ydir = 0;
			break;
		case 2:	// down 
			xdir = 0;
			ydir = -1;
			break;
		case 3:	// left
			xdir = -1;
			ydir = 0;
			break;
		default:
			*iterations = -1;
			return false;
	}
	*tx = x + xdir * (*iterations + 1);
	*ty = y + ydir * (*iterations + 1);
	if (!can_move_minimal(b, x, y, *tx, *ty)) {
		return false;
	}
	*iterations = *iterations + 1;
	return true;
}

static bool horse_movement(board b, int dir, int x, int y, int *tx, int *ty, int *iterations) {
	if (*iterations > 7 || *iterations < 0) {
		*iterations = -1;
		return false;
	}
	int i = *iterations;
	int ydir = (((i / 4) * -2) + 1) * ((((i % 2) + (i / 2)) % 2) + 1);
	i = (*iterations + 2) % 8;
	int xdir = (((i / 4) * -2) + 1) * ((((i % 2) + (i / 2)) % 2) + 1);
	*tx = x + xdir;
	*ty = y + ydir;
	*iterations = *iterations + 1;
	if (!can_move_minimal(b, x, y, *tx, *ty)) {
		return false;
	}
	return true;
}

/*
 * returns true if it can keep going, false if its options have been exhasted
 */
static bool check_piece_movement(board b, chess_piece p, int *dir, int x, int y, int *tx, int *ty, int *iterations) {
	bool ret = false;
	switch (p.pi) {
		case PAWN:
			int sign = (p.c == WHITE) ? -1 : 1;
			return pawn_movement(b, sign, x, y, tx, ty, iterations);
		case ROOK:
			ret = cross_movement(b, *dir, x, y, tx, ty, iterations);
			break;
		case KNIGHT:
			ret = horse_movement(b, *dir, x, y, tx, ty, iterations);
			break;
		case BISHOP:
			ret = diag_movement(b, *dir, x, y, tx, ty, iterations);
			break;
		case QUEEN:
			if (*dir % 2 == 0) {
				ret = cross_movement(b, *dir / 2, x, y, tx, ty, iterations);
			} else {
				ret = diag_movement(b, (*dir - 1) / 2, x, y, tx, ty, iterations);
			}
			break;
		case KING:
			if (*iterations > 0) {
				*iterations = 0;
				*dir = *dir + 1;
			}
			if (*dir % 2 == 0) {
				ret = cross_movement(b, *dir / 2, x, y, tx, ty, iterations);
			} else {
				ret = diag_movement(b, (*dir - 1) / 2, x, y, tx, ty, iterations);
			}
			break;
		default:
			*iterations = -1;
			return false;
	}
	if (*iterations < 0)
		return false;
	if (!ret && p.pi != PAWN && p.pi != KNIGHT) {
		*iterations = 0;
		*dir = *dir + 1;
	}
	return ret;
}

static bool all_movements(board b, board tmp, int x, int y, int kx, int ky) {
	int iterations = 0;
	int dir = 0;
	while (1) {
		int tx, ty;
		chess_piece p = b[y][x];
		while (!check_piece_movement(b, p, &dir, x, y, &tx, &ty, &iterations) && 
				iterations >= 0);
		if (iterations < 0)
			return false;
		memcpy(*tmp, *b, sizeof(board));
		_move(tmp, x, y, tx, ty);
		int tkx = kx, tky = ky;
		if (KING == p.pi) {
			tkx = tx;
			tky = ty;
		}
		if (!incheck(tmp, tkx, tky))
			return true;
	}
}

static size_t what_can_attack_me(board b, color turn, int x, int y, int enemies[16][2]) {
	size_t e = 0;
	// check for pawns
	// only 2 places to check for pawns
	int sign = turn == BLACK ? -1 : 1;
	int py = y - sign;
	int p1x = x - 1;
	int p2x = x + 1;
	if (!out_of_bounds(p1x, py) && PAWN == b[py][p1x].pi && turn != b[py][p1x].c) {
		enemies[e][0] = p1x;
		enemies[e][1] = py;
		e++;
	}
	if (!out_of_bounds(p2x, py) && PAWN == b[py][p2x].pi && turn != b[py][p2x].c) {
		enemies[e][0] = p2x;
		enemies[e][1] = py;
		e++;
	}
	// check for horses
	int iterations = 0;
	int dir = 0;
	do {
		int tx, ty;
		if (!check_piece_movement(b, 
					(chess_piece) { .pi = KNIGHT, .c = turn }, 
					&dir, 
					x, y,
					&tx, &ty, 
					&iterations))
			// if a hypothetical knight on our side in this position can't move to a
			// position, that position is not worth investigating, since
			// that position is either out of bounds or our own piece
			continue;
		if (KNIGHT == b[ty][tx].pi) {
			enemies[e][0] = tx;
			enemies[e][1] = ty;
			e++;
		}
	} while (iterations >= 0);
	// check for everything else with a queen
	iterations = 0;
	dir = 0;
	do {
		int tx, ty;
		if (!check_piece_movement(b, 
					(chess_piece) { .pi = QUEEN, .c = turn }, 
					&dir, 
					x, y,
					&tx, &ty, 
					&iterations))
			// if a hypothetical queen on our side in this position can't move to a
			// position, that position is not worth investigating, since
			// that position is either out of bounds or our own piece
			continue;
		chess_p found = b[ty][tx].pi;
		if (QUEEN == found || (BISHOP == found && tx != x && ty != y) || (ROOK == found && (tx == x || ty == y))) {
			enemies[e][0] = tx;
			enemies[e][1] = ty;
			e++;
		}
	} while (iterations >= 0);
	return e;
}

static void iterate_line(int *x, int *y, int tx, int ty, int iterations) {
	if (*x == tx && *y == ty)
		return;
	if (*x != tx)
		*x += (*x > tx ? -1 : 1) * iterations;
	if (*y != ty)
		*y += (*y > ty ? -1 : 1) * iterations;
}

static bool try_check(board b, board tmp, int x, int y, int tx, int ty, int kx, int ky) {
	if (kx == x && ky == y) {
		kx = tx;
		ky = ty;
	}
	if (!can_move(b, x, y, tx, ty))
		return true;
	memcpy(*tmp, *b, sizeof(board));
	_move(tmp, x, y, tx, ty);
	return incheck(tmp, kx, ky);
}

static bool legal_move_exists(board b, color turn, int kx, int ky, bool check) {
	board tmp;
	if (check) {
		// check all of the king's movements if they get him out of check
		if (all_movements(b, tmp, kx, ky, kx, ky))
			return true;
		// That didn't work, find out what is putting the king in check
		int enemigos[16][2];
		size_t q_enemigos = what_can_attack_me(b, turn, kx, ky, enemigos);
		// since we know the layout of enemigos, we can check the pawns and knights first
		// the only way to stop a pawn or knight is by capturing them, so 
		// see if any of our pieces can take one of these pieces
		// may as well check all the other ones as well
		for (int y = 0; y < BOARD_HEIGHT; ++y) {
			for (int x = 0; x < BOARD_LENGTH; ++x) {
				if (BLANK == b[y][x].pi)
					continue;
				if (b[y][x].c != turn)
					continue;
				for (int i = 0; i < q_enemigos; ++i) {
					// see if it can take any of the enemigos
					// if it can, see if we're still in check
					if (!try_check(b, tmp, x, y, enemigos[i][0], enemigos[i][1], kx, ky))
						return true;
					// better luck next time
				}
			}
		}
		// no luck, try and block one of the enemigos instead
		for (int i = 0; i < q_enemigos; ++i) {
			int tx = enemigos[i][0];
			int ty = enemigos[i][1];
			if (KNIGHT == b[ty][tx].pi || PAWN == b[ty][tx].pi)
				// skip pawns and knights that can't be blocked
				continue;
			for (int y = 0; y < BOARD_HEIGHT; ++y) {
				for (int x = 0; x < BOARD_LENGTH; ++x) {
					if (BLANK == b[y][x].pi)
						continue;
					if (b[y][x].c != turn)
						continue;
					int tmpy = y;
					int tmpx = x;
					int iterations = 1;
					do {
						iterate_line(&tmpx, &tmpy, tx, ty, iterations++);
						if (!try_check(b, tmp, x, y, tmpx, tmpy, kx, ky))
							return true;
					} while (tmpy != ty && tmpx != tx && !out_of_bounds(tmpx, tmpy));
				}
			}
		}
		return false;
	}
	for (int y = 0; y < BOARD_HEIGHT; ++y) {
		for (int x = 0; x < BOARD_LENGTH; ++x) {
			if (BLANK == b[y][x].pi)
				continue;
			if (b[y][x].c != turn)
				continue;
			if (all_movements(b, tmp, x, y, kx, ky))
				return true;
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
	move->piece = PAWN;
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

static int find_x_y(chess_t *chess, move_t *move, bool *kind) {
	if (move->x > -1 && move->y > -1)
		return 1;
	int matches = 0;
	for (int i = 0; i < BOARD_HEIGHT; ++i) {
		for (int j = 0; j < BOARD_LENGTH; ++j) {
			// check if this piece matches the piece we are trying to move
			if (!(chess->b[i][j].pi == move->piece &&
			      chess->b[i][j].c == chess->turn))
				continue;
			// check if this piece can move to the position we are trying to move to 
			if (!can_move(chess->b, j, i, move->tx, move->ty))
				continue;
			// check if this piece matches given position constraints, if any were given
			if ((move->x > -1 && j != move->x) || (move->y > -1 && i != move->y))
				continue;
			matches++;
			if (kind != NULL && matches > 1 && !(matches > 2))
				*kind = move->x == j;
			// assume this is the right piece
			move->x = j;
			move->y = i;
		}
	}
	return matches;
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

static bool test_move(chess_t *chess_board, move_t *move, board buf, int kcopy[2], char *promote, castle_state *state) {
	castle_state col = (chess_board->turn == WHITE) ? W_CASTLE_QUEEN : B_CASTLE_QUEEN;
	if (move->tx != 0)
		col *= 2;
	// one can castle if and only if
	// 1: neither the king nor the rook castling has been moved
	// 2: the line is clear between them
	// 3: the king is not in check
	if ((move->flags & MOVE_CASTLE) &&
			((chess_board->castle & col) == 0 ||
			!check_line(chess_board->b, move->x, move->y, move->tx, move->ty) || 
			NOCOLOR != chess_board->check))
		return false;
	if ((move->flags & MOVE_PROMOTE) && move->piece != PAWN &&
			move->ty != (BOARD_HEIGHT - 1) * swith(chess_board->turn))
		return false;
	memcpy(*buf, *chess_board->b, sizeof chess_board->b);
	// make the move on the temporary board
	bool taken = _move(buf, move->x, move->y, move->tx, move->ty);
	if ((move->flags & MOVE_CAPTURE) && !taken)
		return false;
	if (taken)
		move->flags |= MOVE_CAPTURE;
	memcpy(kcopy, chess_board->kpos[chess_board->turn], 2 * sizeof *kcopy);
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
	if (move->flags & MOVE_CASTLE) {
		// we are castling, move the rook
		castle_state tstate = (chess_board->turn == WHITE) ? W_CASTLE_QUEEN : B_CASTLE_QUEEN;
		int x = (tstate == col) ? 0 : BOARD_LENGTH - 1;
		int tx = (tstate == col) ? 3 : BOARD_LENGTH - 3;
		_move(buf, x, move->y, tx, move->ty);
	}
	*state = cstate;
	if (move->flags & MOVE_PROMOTE)
		return promotion(buf, move, promote);
	return true;
}

size_t unparse_movement(char buf[10], move_t *move, chess_t *chess) {
	char *piece = unparse_piece(move->piece);
	int x = move->x;
	int y = move->y;
	int tx = move->tx;
	int ty = move->ty;
	move->x = -1;
	move->y = -1;
	bool kind = false;
	int matches = find_x_y(chess, move, &kind);
	char d_rank = BOARD_HEIGHT - y + '0';
	char d_file = 'a' + x;
	char t_rank = BOARD_HEIGHT - ty + '0';
	char t_file = 'a' + tx;
	if (matches == 1) {
		d_rank = 0;
		d_file = 0;
	} else if (matches == 2) {
		if (kind) {
			// xs match -> on same file, provide rank information
			d_file = 0;
		} else {
			// ys match -> on same rank, provide file
			d_rank = 0;
		}
	}
	char disambig[3];
	snprintf(disambig, 3, "%c%c", d_file, d_rank);
	if (0 == *disambig) {
		disambig[0] = disambig[1];
		disambig[1] = 0;
	}
	char take[2];
	snprintf(take, 2, "%c", move->flags & MOVE_CAPTURE ? 'x' : '\0');
	char end[2];
	snprintf(end, 2, "%c", (move->flags & MOVE_CHECK) ? '+' : ((move->flags & MOVE_MATE) ? '#' : '\0'));
	// put everything together
	return snprintf(buf, 10, "%s%s%s%c%c%s ", piece, disambig, take, t_file, t_rank, end);
}

unsigned int ensure_space(char **arr, unsigned int cur_size, unsigned int req_size) {
	if (cur_size <= req_size)
		return cur_size;
	while ((cur_size *= 2) < req_size);
	*arr = realloc(*arr, cur_size);
	return cur_size;
}

void append_history(chess_t *chess, move_t *move) {
	char buf[10];
	size_t move_len = unparse_movement(buf, move, chess);
	chess->h_len = ensure_space(&chess->history, 
			chess->h_len, 
			chess->h_end + move_len + (WHITE == chess->turn) ? 8 : 0);
	if (WHITE == chess->turn) {
		char num[8];
		int num_size = snprintf(num, 8, "%u. ", chess->moves / 2 + 1);
		memcpy(chess->history + chess->h_end - 1, num, num_size + 1);
		chess->h_end += num_size;
	}
	memcpy(chess->history + chess->h_end - 1, buf, move_len + 1);
	chess->h_end += move_len;
}

// returns -1 if error,
// 0 if normal
// 1 if in check
// 2 if checkmate
char move(chess_t *chess_board, char *notation) {
	move_t move;
	char *promote = NULL;
	if (!parse_movement(notation, chess_board->turn, &move, promote)) {
		fprintf(stderr, "notation syntax error: %s\n", notation);
		return CHESS_ERR;
	}
	int matches = find_x_y(chess_board, &move, NULL);
	if (matches > 1) {
		fprintf(stderr, "ambiguous input: %s\n", notation);
		return CHESS_ERR;
	}
	if (matches < 1) {
		fprintf(stderr, "no piece can achieve %s\n", notation);
		return CHESS_ERR;
	}
	board tmp_b;
	castle_state cstate;
	int kcopy[2];
	if (!test_move(chess_board, &move, tmp_b, kcopy, promote, &cstate)) {
		fprintf(stderr, "unable to make %s\n", notation);
		return CHESS_ERR;
	}
	color turn  = swith(chess_board->turn);
	color check = incheck(tmp_b, chess_board->kpos[turn][0], chess_board->kpos[turn][1])
		? turn
		: NOCOLOR;
	char ret = CHESS_NORMAL;
	if (legal_move_exists(tmp_b, turn, chess_board->kpos[turn][0], chess_board->kpos[turn][1], check != NOCOLOR)) {
		// if the user says this move results in mate, return error
		if (move.flags & MOVE_MATE)
			return CHESS_ERR;
		// if the user says this move results in check and it does not, return error
		if (NOCOLOR == check && move.flags & MOVE_CHECK)
			return CHESS_ERR;
		if (NOCOLOR != check) {
			ret = CHESS_CHECK;
			move.flags |= MOVE_CHECK;
		}
	} else {
		// no legal move exists
		if (NOCOLOR == check && move.flags & MOVE_MATE)
			return CHESS_ERR;
		ret = (NOCOLOR == check) ? CHESS_STALE : CHESS_MATE;
		if (CHESS_MATE == ret)
			move.flags |= MOVE_MATE;
	}

	// TODO record this move
	chess_board->moves++;
	append_history(chess_board, &move);

	memcpy(*chess_board->b, *tmp_b, sizeof chess_board->b);
	chess_board->kpos[chess_board->turn][0] = kcopy[0];
	chess_board->kpos[chess_board->turn][1] = kcopy[1];
	chess_board->castle &= ~cstate;
	chess_board->turn = turn;
	chess_board->check = check;


	return ret;
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
	chess_board->moves = 0;
	chess_board->h_len = 16;
	chess_board->h_end = 1;
	chess_board->history = malloc(chess_board->h_len * sizeof *(chess_board->history));
	memset(chess_board->history, 0, chess_board->h_len * sizeof *(chess_board->history));
}

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
