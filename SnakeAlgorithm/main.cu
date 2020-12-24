#include <rapid/rapid.h>

#include "game.h"
#include "bot.h"

int main()
{
	std::cout << "Starting snake\n";

	size_t blocksX = 51; // 20;
	size_t blocksY = 51; // 20;
	size_t windowWidth = 1000;
	size_t windowHeight = 1000;
	unsigned int updateFrames = 1; // 40;
	int deathSaveTimer = 20; // 10;

	LevelDesigner designer(blocksX, blocksY, windowWidth, windowHeight);
	designer.start();


	SnakeGame game(blocksX, blocksY, windowWidth, windowHeight, 0, updateFrames, deathSaveTimer);

	game.board = designer.getBoard();
	// game.targetFrameRate = 288;
	game.limitFrameRate = false;

	game.initialize();
	game.setup();

	SnakeBot bot(&game);

	while (game.isOpen())
	{
		if (game.frameCount % game.updateMod == 0)
			bot.makeMove();

		game.update();
	}

	std::cout << "Score: " << game.snakePositions.size() << "\n";

	return 0;
}
