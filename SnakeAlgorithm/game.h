#pragma once

#include <rapid/rapid.h>
#include <random>

inline double randomDouble()
{
	// Random double in range [0, 1)

	static std::uniform_real_distribution<double> distribution(0., 1.);
	static std::mt19937 generator(TIME * 100000);
	return distribution(generator);
}

inline double randomDouble(const double &min, const double &max)
{
	// Random double in range [min, max)

	return min + (max - min) * randomDouble();
}

inline double clamp(const double &x, const double &min, const double &max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

class SnakeGame : public rapid::RapidGraphics
{
public:
	size_t blocksX;
	size_t blocksY;
	double blockWidth;
	double blockHeight;
	double blockDensity;

	int deathPause = -1;

	std::vector<std::vector<bool>> board;

	std::vector<rapid::Vec2<int>> snakePositions;
	std::string direction;

	rapid::Vec2<int> foodPos;
	bool shouldPlaceFood = false;

	unsigned int updateMod;
	int difficulty = 3;

	unsigned char snakeR = 100;
	unsigned char snakeG = 200;
	unsigned char snakeB = 255;

	SnakeGame(size_t x, size_t y, size_t w, size_t h, double bd, unsigned int fmod, int difficultyLevel)
	{
		blocksX = x;
		blocksY = y;

		blockWidth = (double) w / (double) blocksX;
		blockHeight = (double) h / (double) blocksY;

		blockDensity = bd;

		board.resize(blocksY);

		for (size_t i = 0; i < blocksY; i++)
		{
			board[i].resize(blocksX);
		}

		updateMod = fmod;
		difficulty = rapid::rapidMax(difficultyLevel, 1);

		create(w, h, "Snake Game");
	}

	bool setup() override
	{
		// Initialize the board state
		std::mt19937_64 twister(TIME * 10000000);

		std::uniform_real_distribution<> blockDistribution(0, 1);

		for (size_t i = 0; i < blocksY; i++)
		{
			for (size_t j = 0; j < blocksX; j++)
			{
				double val = blockDistribution(twister);

				if (val < blockDensity)
				{
					board[i][j] = true;
				}
			}
		}

		std::uniform_int_distribution<> bodyDistributionX(0, blocksX - 1);
		std::uniform_int_distribution<> bodyDistributionY(0, blocksY - 1);

		bool placeSnake = true;
		int headX, headY;
		while (placeSnake)
		{
			headX = bodyDistributionX(twister);
			headY = bodyDistributionY(twister);

			placeSnake = false;

			for (size_t i = 0; i < blocksY; i++)
			{
				for (size_t j = 0; j < blocksX; j++)
				{
					if (board[i][j] && j == headX && i == headY)
						placeSnake = true;
				}
			}
		}

		snakePositions.emplace_back(headX, headY);

		placeFood();

		return true;
	}

	bool draw() override
	{
		for (size_t i = 0; i < blocksY; i++)
		{
			for (size_t j = 0; j < blocksX; j++)
			{
				stroke(0);
				strokeWeight(0);

				if (board[i][j])
				{
					fill(255, 50, 50);
					rect(j * blockWidth, i * blockHeight, blockWidth, blockHeight);
				}
				else
					fill(50);

				rect(j * blockWidth, i * blockHeight, blockWidth, blockHeight);
			}
		}

		strokeWeight(1);

		for (int i = 0; i < blocksX; i++)
			line(i * blockWidth, 0, i * blockWidth, height);

		for (int i = 0; i < blocksY; i++)
			line(0, i * blockHeight, width, i * blockHeight);

		if (frameCount % updateMod == 0)
		{
			updateSnake();
		}

		strokeWeight(0);

		for (size_t i = 0; i < snakePositions.size(); i++)
		{
			fill(snakeR, snakeG, snakeB);

			if (i == 0)
				fill(255 - snakeR, 255 - snakeG, 255 - snakeB);

			// rect(snakePositions[i].x * blockWidth, snakePositions[i].y * blockHeight, blockWidth, blockHeight);

			// Draw a rectangle from the current segment to the next to get a continuous snake that is less than a block wide

			double x1, y1, x2, y2, rectWidth, rectHeight;
			x1 = snakePositions[i].x * blockWidth + (blockWidth / 10);
			y1 = snakePositions[i].y * blockHeight + (blockHeight / 10);

			if (i < snakePositions.size() - 1)
			{
				if (snakePositions[i].y <= snakePositions[i + 1].y)
					y2 = (snakePositions[i + 1].y + 1) * blockHeight - (blockHeight / 10);
				else
				{
					y1 = snakePositions[i + 1].y * blockHeight + (blockHeight / 10);
					y2 = (snakePositions[i].y + 1) * blockHeight - (blockHeight / 10);
				}

				if (snakePositions[i].x <= snakePositions[i + 1].x)
					x2 = (snakePositions[i + 1].x + 1) * blockWidth - (blockWidth / 10);
				else
				{
					x1 = snakePositions[i + 1].x * blockWidth + (blockWidth / 10);
					x2 = (snakePositions[i].x + 1) * blockWidth - (blockWidth / 10);
				}

				rectWidth = x2 - x1;
				rectHeight = y2 - y1;
			}
			else
			{
				rectWidth = blockWidth - (blockWidth / 5);
				rectHeight = blockHeight - (blockHeight / 5);
			}

			strokeWeight(0);
			rect(x1, y1, rectWidth, rectHeight);
		}

		if (frameCount % updateMod * 2 < updateMod)
			fill(100, 100, 255);
		else
			fill(175, 175, 230);

		rect(foodPos.x * blockWidth, foodPos.y * blockHeight, blockWidth, blockHeight);

		return true;
	}

	void updateSnake()
	{
		if (shouldPlaceFood)
			placeFood();

		if (deathPause == -1 && checkDeathOnUpdate())
			deathPause = difficulty;

		if (!checkDeathOnUpdate())
			deathPause = -1;

		if (deathPause == -1)
		{
			for (size_t i = snakePositions.size(); i > 1; i--)
			{
				snakePositions[i - 1] = snakePositions[i - 2];
			}

			if (direction == "UP")
				snakePositions[0].y--;
			if (direction == "DOWN")
				snakePositions[0].y++;
			if (direction == "LEFT")
				snakePositions[0].x--;
			if (direction == "RIGHT")
				snakePositions[0].x++;
		}
		else
			deathPause--;

		if (deathPause == 0)
			rapid::RapidError("Game Over!", "Well played. Try again!").display();

		// Food collision
		if (snakePositions[0].x == foodPos.x && snakePositions[0].y == foodPos.y)
		{
			snakePositions.emplace_back(snakePositions[snakePositions.size() - 1]);

			shouldPlaceFood = true;
		}
	}

	inline bool checkDeathOnUpdate()
	{
		bool result = false;

		// Update head position
		if (direction == "UP")
			snakePositions[0].y--;
		if (direction == "DOWN")
			snakePositions[0].y++;
		if (direction == "LEFT")
			snakePositions[0].x--;
		if (direction == "RIGHT")
			snakePositions[0].x++;

		if (snakePositions[0].x < 0 || snakePositions[0].x >= blocksX || snakePositions[0].y < 0 || snakePositions[0].y >= blocksY)
			result = true;

		// Collisions with snake body
		for (size_t i = 1; i < snakePositions.size(); i++)
			if (snakePositions[0].x == snakePositions[i].x && snakePositions[0].y == snakePositions[i].y)
				result = true;

		// Collisions with the board
		for (size_t i = 0; i < blocksY; i++)
			for (size_t j = 0; j < blocksX; j++)
				if (snakePositions[0].x == j && snakePositions[0].y == i && board[i][j])
					result = true;

		// Reset head position
		if (direction == "UP")
			snakePositions[0].y++;
		if (direction == "DOWN")
			snakePositions[0].y--;
		if (direction == "LEFT")
			snakePositions[0].x++;
		if (direction == "RIGHT")
			snakePositions[0].x--;

		return result;
	}

	void placeFood()
	{
		std::mt19937_64 twister(TIME * 10000000);
		std::uniform_int_distribution<> bodyDistributionX(0, blocksX - 1);
		std::uniform_int_distribution<> bodyDistributionY(0, blocksY - 1);

		bool placeFood = true;
		int foodX, foodY;
		while (placeFood)
		{
			foodX = bodyDistributionX(twister);
			foodY = bodyDistributionY(twister);

			placeFood = false;

			for (size_t i = 0; i < blocksY; i++)
				for (size_t j = 0; j < blocksX; j++)
					if (board[i][j] && j == foodX && i == foodY)
						placeFood = true;

			for (const auto &body : snakePositions)
				if (body.x == foodX && body.y == foodY)
					placeFood = true;
		}

		foodPos = {foodX, foodY};
		shouldPlaceFood = false;
	}

	bool onKeyPress(rapid::keyType key, rapid::keyAction action, const std::vector<rapid::keyModifier> &mods) override
	{
		if (action == rapid::ACTION_PRESS)
		{
			if (key == rapid::KEY_W || key == rapid::KEY_UP)
				direction = "UP";
			if (key == rapid::KEY_A || key == rapid::KEY_LEFT)
				direction = "LEFT";
			if (key == rapid::KEY_S || key == rapid::KEY_DOWN)
				direction = "DOWN";
			if (key == rapid::KEY_D || key == rapid::KEY_RIGHT)
				direction = "RIGHT";
		}

		return true;
	}

	void setDirection(const std::string &dir)
	{
		if (!dir.empty())
		{
			if (!(dir == "UP" ||
				dir == "DOWN" ||
				dir == "LEFT" ||
				dir == "RIGHT"))
			{
				rapid::RapidError("Direction Error", "Unknown direction given to snake: " + dir).display();
			}

			direction = dir;
		}
	}
};

class LevelDesigner : public rapid::RapidGraphics
{
public:
	std::vector<std::vector<bool>> board;
	size_t blocksX;
	size_t blocksY;
	double blockWidth;
	double blockHeight;

	LevelDesigner(size_t x, size_t y, size_t w, size_t h)
	{
		blocksX = x;
		blocksY = y;

		blockWidth = (double) w / (double) blocksX;
		blockHeight = (double) h / (double) blocksY;

		board.resize(blocksY);

		for (size_t i = 0; i < blocksY; i++)
		{
			board[i].resize(blocksX);
		}

		create(w, h, "Snake Game Level Designer");
	}

	bool setup() override
	{
		targetFrameRate = 288;

		if (blocksX == 50 && blocksY == 50)
		{
			board = {{false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, true, true, false, false, false, true, true, false, true, true, true, true, false, true, true, true, true, false, true, true, true, true, true, false, true, true, true, true, false, true, true, true, true, false, true, true, false, false, false, true, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, true, false, false, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, true, true, false, false, false, true, false, false, false, true, true, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, true, true, false, true, true, true, false, false, false, true, true, true, false, false, false, true, false, false, false, true, true, true, false, false, false, true, true, true, false, true, true, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, true, true, false, false, false, true, false, false, false, true, true, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, true, false, false, false, true, true, true, false, false, false, false, false, false, false, true, true, true, false, false, false, true, false, false, false, false, true, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, false, false, false, false},
					 {false, false, false, false, false, true, true, true, false, false, false, false, false, false, false, true, false, false, false, true, true, false, true, true, false, false, false, true, true, false, true, true, false, false, false, true, false, false, false, false, false, false, false, true, true, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, true, true, false, false, false, false, false, false, false, true, true, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, true, true, false, true, true, true, false, false, false, true, true, true, false, false, false, true, false, false, false, true, true, true, false, false, false, true, true, true, false, true, true, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, true, true, false, false, false, true, false, false, false, true, true, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, true, false, false, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false, false, false},
					 {false, false, false, false, false, true, true, false, false, false, true, true, false, true, true, true, true, false, true, true, true, true, false, true, true, true, true, true, false, true, true, true, true, false, true, true, true, true, false, true, true, false, false, false, true, true, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
					 {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
			};
		}

		return true;
	}

	bool draw() override
	{
		for (size_t i = 0; i < blocksY; i++)
		{
			for (size_t j = 0; j < blocksX; j++)
			{
				stroke(0);
				strokeWeight(0);

				if (board[i][j])
					fill(255, 50, 50);
				else
					fill(50);

				rect(j * blockWidth, i * blockHeight, blockWidth, blockHeight);
			}
		}

		strokeWeight(1);

		for (int i = 0; i < blocksX; i++)
			line(i * blockWidth, 0, i * blockWidth, height);

		for (int i = 0; i < blocksY; i++)
			line(0, i * blockHeight, width, i * blockHeight);

		if (mousePressed(0) || mousePressed(1))
		{
			// Set the current position to the selected mode

			auto x = (int) (mouseX / blockWidth);
			auto y = (int) (mouseY / blockHeight);

			if (x < blocksX && y < blocksY)
				board[y][x] = mousePressed(0) ? true : false;
		}

		return true;
	}

	auto getBoard() const
	{
		std::cout << "{";
		for (size_t i = 0; i < blocksY; i++)
		{
			std::cout << "{";
			for (size_t j = 0; j < blocksX; j++)
			{
				std::cout << (board[i][j] ? "true" : "false");

				if (j + 1 < blocksX)
					std::cout << ", ";
			}
			std::cout << "}";

			if (i + 1 < blocksY)
				std::cout << ", ";

			std::cout << "\n";
		}
		std::cout << "}\n";

		return board;
	}
};
