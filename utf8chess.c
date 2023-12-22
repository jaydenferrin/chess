#include "chess.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <string.h>

#define BUF_LEN 64

void print_board(board);
void change_term(void);
void reset_term(void);

struct termios *orig_info = NULL;

int main(void) {
	chess_t game;
	char buf[BUF_LEN];

	reset(&game);

//	atexit(reset_term);
//	signal(SIGINT, exit);
//	change_term();

	while (1) {
		print_board(game.b);
		printf("%s's move: "
				"                \033[16D", 
				print_color(game.turn));
		if (NULL == fgets(buf, BUF_LEN, stdin)) {
			printf("\n");
			return 0;
		}
		strtok(buf, "\n");
		// buf has the move the player wants to make
		if (move(&game, buf))
			printf("\033[1A");
		printf("\r\033[10A");
	}

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

void change_term(void) {            
	if (NULL == orig_info) {
		// store original terminal state
		orig_info = malloc(sizeof *orig_info);
		// copy current terminal state
		struct termios info;
		tcgetattr(STDIN_FILENO, orig_info);
		info = *orig_info;
		// disable canonical mode and echo
		info.c_lflag &= ~(ICANON | ECHO);
		// idk lol
		info.c_cc[VMIN] = 1;
		info.c_cc[VTIME] = 0;
		// set terminal state to altered state
		tcsetattr(STDIN_FILENO, TCSANOW, &info);
	}
}

void reset_term(void) {
	if (NULL != orig_info) {
		tcsetattr(STDIN_FILENO, TCSANOW, orig_info);
		free(orig_info);
		orig_info = NULL;
	}
}

