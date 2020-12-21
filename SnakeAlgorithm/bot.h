#pragma once

#include "game.h"

class Spot
{
public:
	double x, y;
	double f, g, h;
	std::vector<Spot *> neighbors;
	bool wall;

	Spot *previous = nullptr;

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
	std::vector<std::string> currentPath;
	size_t prevSnakeLen = 1;
	size_t updates = 0;
	std::string mode = "PATH";

	SnakeBot(SnakeGame *game_) : game(game_)
	{}

	void makeMove()
	{
		// Find the path to the food
		currentPath = shortestPathToFood();

		if (!currentPath.empty())
		{
			// Check if it is safe to path
			if (currentPath[0] == "INVALID")
				mode = "SAFE";
			else
				mode = "PATH";

			if (mode == "PATH")
			{
				game->setDirection(currentPath[0]);
				currentPath.erase(currentPath.begin());
			}
			else if (mode == "SAFE")
			{

			}
		}

		updates++;
	}

	std::vector<std::string> shortestPathToFood() const
	{
		auto grid = convertBoardToSpots();

		std::vector<Spot *> openSet;
		std::vector<Spot *> closedSet;
		Spot *start = &grid[game->snakePositions[0].y][game->snakePositions[0].x];
		Spot *end = &grid[game->foodPos.y][game->foodPos.x];

		openSet.push_back(start);

		bool pathFound = false;

		while (!pathFound)
		{
			if (!openSet.empty())
			{
				// Find the closest cell in the open set
				size_t index = 0;
				for (size_t i = 0; i < openSet.size(); i++)
					if (openSet[i]->f < openSet[index]->f)
						index = i;

				auto current = openSet[index];

				// Check if the end is reached
				if (current == end)
				{
					// Found the food, so stop searching

					// Calculate the path
					std::vector<std::string> path;

					auto temp = current;

					while (temp->previous)
					{
						// Get the direction as a string
						std::string dir;

						if (temp->x == temp->previous->x)
						{
							// x coordinate is the same
							if (temp->previous->y < temp->y)
								dir = "DOWN";
							else
								dir = "UP";
						}
						else
						{
							// y coordinate is the same
							if (temp->previous->x < temp->x)
								dir = "RIGHT";
							else
								dir = "LEFT";
						}

						// Log the value and recurse backwards down the tree
						path.insert(path.begin(), dir);
						temp = temp->previous;
					}

					return path;
				}

				// Current value did not complete the path
				// Remove it from open set and add it to the closed set
				openSet.erase(std::remove(openSet.begin(), openSet.end(), current), openSet.end());
				closedSet.emplace_back(current);

				// Check current's neighbors and continue from there
				for (const auto &cell : current->neighbors)
				{
					// Check that this cell is not in the closed set and that it is not a wall
					if (std::find(closedSet.begin(), closedSet.end(), cell) == closedSet.end() && !cell->wall)
					{
						auto tempG = current->g + 1;
						bool newPath = false;

						if (std::find(closedSet.begin(), closedSet.end(), cell) != closedSet.end())
						{
							// Cell is in the open set

							if (tempG < cell->g)
							{
								// Found shorter path to this cell
								cell->g = tempG;
								newPath = true;
							}
						}
						else
						{
							// Cell has not yet been visited
							cell->g = tempG;
							openSet.emplace_back(cell);
							newPath = true;
						}

						if (newPath)
						{
							cell->h = heuristic(cell, end);
							cell->f = cell->g + cell->h;
							cell->previous = current;
						}
					}
				}
			} // Keep searching for a path
			else
			{
				// There was no path
				return {"INVALID"};
			}
		}

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

	double heuristic(const Spot *a, const Spot *b) const
	{
		double dx = b->x - a->x;
		double dy = b->y - a->y;

		return sqrt((dx * dx) + (dy * dy));
	}
};
