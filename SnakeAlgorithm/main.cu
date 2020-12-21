#include <rapid/rapid.h>

#include "game.h"
#include "bot.h"

int main()
{
	std::cout << "Starting snake\n";

	size_t blocksX = 20;
	size_t blocksY = 20;
	size_t windowWidth = 700;
	size_t windowHeight = 700;
	double blockDensity = 0;
	unsigned int updateFrames = 40;
	int deathSaveTimer = 10;

	LevelDesigner designer(blocksX, blocksY, windowWidth, windowHeight);
	designer.start();


	SnakeGame game(blocksX, blocksY, windowWidth, windowHeight, blockDensity, updateFrames, deathSaveTimer);

	game.board = designer.getBoard();
	game.targetFrameRate = 288;

	game.initialize();
	game.setup();

	SnakeBot bot(&game);

	while (game.isOpen())
	{
		game.update();

		// if (game.frameCount % 20 == 0)
		// 	bot.makeMove();
	}

	std::cout << "Score: " << game.snakePositions.size() << "\n";

	return 0;
}
