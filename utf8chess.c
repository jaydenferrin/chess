#include "chess.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define BUF_LEN 64

void print_board(board);

struct termios *orig_info = NULL;

int main(void) {
	chess_t game;
	char buf[BUF_LEN];
	int state = 0;

	reset(&game);

	while (1) {
		print_board(game.b);
		if (state > 1) {
			printf("\n");
			break;
		}
		printf(         "                                \033[32D\r"
				//"                                \033[32D\033[1A\r"
				"%s's move: ", 
				print_color(game.turn));
		if (NULL == fgets(buf, BUF_LEN, stdin))
			break;
		strtok(buf, "\n");
		// buf has the move the player wants to make
		printf("                                \033[32D\r");
		state = move(&game, buf);
		if (state)
			printf("\033[1A");
		printf("\r\033[10A");
	}

	printf("\n");

	return 0;
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
