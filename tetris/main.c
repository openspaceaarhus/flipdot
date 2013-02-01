#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "game.h"

const char usage_str[] =
	"Usage:\n"
	"  tetris [-h] port [-h]\n"
	;

char *serial_port = NULL;

int main(int argc, char *argv[])
{
	int optc;
	int lose = 0;

	/* Game object */
	StcGame game;
  
	while(EOF != (optc = getopt(argc, argv, "h"))) {
		switch(optc) {
		case 'h':
			printf("%s", usage_str);
			lose++;
			break;
		default:
			lose++;
			break;
		}
	}

	if(lose)
		exit(1);

	if(optind >= argc) {
		fprintf(stderr, "Missing serial port\n");
		exit(1);
	} else {
		serial_port = strdup(argv[optind]);
	}

	/* Start the game */
	gameInit(&game);

	/* Loop until some error happens or the user quits */
	while (game.errorCode == ERROR_NONE) {
		gameUpdate(&game);
	}

	/* Game was interrupted or an error happened, end the game */
	gameEnd(&game);

	/*  Return to the system */
	return game.errorCode;

	return 0;
}
