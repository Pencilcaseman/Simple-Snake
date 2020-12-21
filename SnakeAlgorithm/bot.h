#pragma once

#include "game.h"

class Spot
{
public:
	double x, y;
	double f, g, h;
	std::vector<Spot *> neighbors;
	bool wall;

	// Spot *previous = nullptr;

	Spot() = default;

	Spot(const double &xx, const double &yy, const double &ff, const double &gg, const double &hh, const bool &isWall)
	{
		x = xx;
		y = yy;
		f = ff;
		g = gg;
		h = hh;

		wall = isWall;
	}

	void addNeighbors(std::vector<std::vector<Spot>> &grid)
	{
		if (x > 0)
			neighbors.emplace_back(&grid[y][x - 1]);
		if (x < grid[0].size() - 1)
			neighbors.emplace_back(&grid[y][x + 1]);
		if (y > 0)
			neighbors.emplace_back(&grid[y - 1][x]);
		if (y < grid.size() - 1)
			neighbors.emplace_back(&grid[y + 1][x]);
	}
};

class SnakeBot
{
public:
	SnakeGame *game;

	SnakeBot(SnakeGame *game_) : game(game_)
	{}

	void makeMove() const
	{
		game->setDirection("UP");

		auto path = shortestPathToFood();
	}

	std::vector<std::string> shortestPathToFood() const
	{
		auto grid = convertBoardToSpots();

		std::vector<Spot *> openSet;
		std::vector<Spot *> closedSet;
		Spot *start = &grid[game->snakePositions[0].y][game->snakePositions[0].x];
		Spot *end = &grid[game->foodPos.y][game->foodPos.x];

		openSet.push_back(start);

		return {"hello"};
	}

	std::vector<std::vector<Spot>> convertBoardToSpots() const
	{
		std::vector<std::vector<Spot>> result;
		result.resize(game->blocksY);

		for (size_t i = 0; i < game->blocksY; i++)
			result[i].resize(game->blocksX);

		// Initialize the spots
		for (size_t i = 0; i < game->blocksY; i++)
		{
			for (size_t j = 0; j < game->blocksX; j++)
			{
				result[i][j] = Spot((double) j, (double) i, 0, 0, 0, game->board[i][j]);
			}
		}

		for (const auto &pos : game->snakePositions)
		{
			if (&pos != &game->snakePositions[0])
				result[pos.y][pos.x].wall = true;
		}

		for (size_t i = 0; i < game->blocksY; i++)
		{
			for (size_t j = 0; j < game->blocksX; j++)
			{
				result[i][j].addNeighbors(result);
			}
		}

		return result;
	}
};
