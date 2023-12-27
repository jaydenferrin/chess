#include "chess.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define BUF_LEN 64

void print_board(board);
char ***parse_args(int argc, char *argv[]);
void free_triple(char ***ptr, size_t size);

struct termios *orig_info = NULL;

int main(int argc, char *argv[]) {
	chess_t game;
	char buf[BUF_LEN];
	chess_return state = 0;

	reset(&game);

	if (argc > 1) {
		char ***notation = parse_args(argc, argv);
		if (NULL == notation)
			return 1;
		for (int i = 0; i < argc / 2; ++i) {
			if (NULL == notation[i])
				break;
			if (game.turn != WHITE)
				break;
			if (NULL == notation[i][0])
				break;
			if (-1 == move(&game, notation[i][0]))
				break;
			if (game.turn != BLACK)
				break;
			if (NULL == notation[i][1])
				break;
			if (-1 == move(&game, notation[i][1]))
				break;
		}
		free_triple(notation, argc / 2);
	}

	while (1) {
		print_board(game.b);
		if (state >= CHESS_END) {
			printf("\n");
			break;
		}
		printf(         "                                                                                "
				"\033[80D\r"
				//"                                \033[32D\033[1A\r"
				"%s's move: ", 
				print_color(game.turn));
		if (NULL == fgets(buf, BUF_LEN, stdin))
			break;
		strtok(buf, "\n");
		// buf has the move the player wants to make
		printf("                                                                                "
				"\033[80D\r");
		state = move(&game, buf);
		switch (state) {
			case CHESS_ERR:
				printf("unknown error occurred.\n");
				break;
			case CHESS_ERR_PROMISE:
				printf("Move %s included parts that could not be achieved\n", buf);
				break;
			case CHESS_ERR_ILLEGAL:
				printf("%s is an illegal move\n", buf);
				break;
			case CHESS_ERR_NOAVAIL:
				printf("No piece is able to achieve %s\n", buf);
				break;
			case CHESS_ERR_AMBIG:
				printf("%s is too ambiguous\n", buf);
				break;
			case CHESS_ERR_PARSE:
				printf("%s is not a valid move\n", buf);
				break;
			case CHESS_CHECK:
				printf("%s in check!\n", print_color(game.check));
				break;
			case CHESS_MATE:
				printf("%s wins by checkmate!\n", print_color(swith(game.check)));
				break;
			case CHESS_STALE:
				printf("Game ends in stalemate\n");
				break;
			default:
				break;
		}
#ifndef DEBUG
		if (state)
			printf("\033[1A");
		printf("\r\033[10A");
#endif
	}

	printf("\n");
	printf("%s\n", game.history);

	return 0;
}

void free_triple(char ***ptr, size_t size) {
	for (int i = 0; i < size; ++i) {
		free(ptr[i]);
	}
	free(ptr);
}

char ***parse_args(int argc, char *argv[]) {
	char ***ret = malloc((argc / 2) * sizeof *ret);
	memset(ret, 0, argc / 2);
	for (int i = 0; i < argc / 2; ++i) {
		ret[i] = malloc(2 * sizeof **ret);
		ret[i][0] = 0;
		ret[i][1] = 0;
	}
	char buf[16];
	for (int i = 1; i < argc; ++i) {
		int j = (i - 1) % 3;
		switch (j) {
			case 0:
				snprintf(buf, 16, "%d.", (i - 1) / 3 + 1);
				if (strcmp(buf, argv[i]) != 0) {
					free_triple(ret, argc / 2);
					return NULL;
				}
				break;
			case 1:
				ret[(i - 1) / 3][0] = argv[i];
				break;
			default:
				ret[(i - 1) / 3][1] = argv[i];
				break;
		}
	}
	return ret;
}


void print_board(board b) {
	static const char pieces[2][10][8] = {{" ", " ", "♙", "♖", "♘", "♗", "♕", "♔"},
					      {" ", " ", "♟", "♜", "♞", "♝", "♛", "♚"}};
	static const int colors[] = {47, 44};
	for (int i = 0; i < BOARD_LENGTH; ++i) {
		printf("%d ", BOARD_LENGTH - i);
		for (int j = 0; j < BOARD_HEIGHT; ++j) {
			int p = b[i][j].pi + 2;
			int c = b[i][j].c;
			if (c < 0) {
				c = 0;
				p = 0;
			}
			printf("\033[%d;30m%s ", colors[(i + j) % 2], pieces[c][p]);
		}
		printf("\033[0m\n");
	}
	printf("  ");
	for (int i = 0; i < BOARD_LENGTH; ++i)
		printf("%c ", 'a' + i);
	printf("\n");
}
