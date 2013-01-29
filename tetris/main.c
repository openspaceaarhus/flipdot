#include "game.h"

int main(int argc, char *argv[])
{
	/* Game object */
	StcGame game;
  
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
