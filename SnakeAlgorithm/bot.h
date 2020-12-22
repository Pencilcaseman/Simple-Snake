#pragma once

#include "game.h"

class Spot
{
public:
	double x = 0, y = 0;
	double f = 0, g = 0, h = 0;
	std::vector<Spot *> neighbors{};
	bool wall = false;

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

		neighbors.clear();
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

	void addNeighborsConditional(std::vector<std::vector<Spot>> &grid)
	{
		if (x > 0 && !grid[y][x - 1].wall && grid[y][x - 1].f != 1)
			neighbors.emplace_back(&grid[y][x - 1]);

		if (x < grid[0].size() - 1 && !grid[y][x + 1].wall && grid[y][x + 1].f != 1)
			neighbors.emplace_back(&grid[y][x + 1]);

		if (y > 0 && !grid[y - 1][x].wall && grid[y - 1][x].f != 1)
			neighbors.emplace_back(&grid[y - 1][x]);

		if (y < grid.size() - 1 && !grid[y + 1][x].wall && grid[y + 1][x].f != 1)
			neighbors.emplace_back(&grid[y + 1][x]);
	}
};

class SnakeBot
{
public:
	SnakeGame *game;

	std::vector<std::string> currentPath;
	std::vector<std::string> safePath;

	size_t prevSnakeLen = 1;
	size_t updates = 0;
	std::string mode = "PATH";

	SnakeBot(SnakeGame *game_) : game(game_)
	{}

	void makeMove()
	{
		if (mode != "ULTRA_SAFE")
		{
			// Find the path to the food
			currentPath = pathFind(game->snakePositions[0].x, game->snakePositions[0].y, game->foodPos.x, game->foodPos.y, convertBoardToSpots());

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
					std::string newDirection = "NONE";
					auto prevDir = game->direction;

					// Just try to survive

					game->direction = "UP";
					if (!game->checkDeathOnUpdate())
						newDirection = "UP";

					game->direction = "DOWN";
					if (!game->checkDeathOnUpdate())
						newDirection = "DOWN";

					game->direction = "LEFT";
					if (!game->checkDeathOnUpdate())
						newDirection = "LEFT";

					game->direction = "RIGHT";
					if (!game->checkDeathOnUpdate())
						newDirection = "RIGHT";

					game->setDirection(prevDir);

					if (newDirection != "NONE")
						game->setDirection(newDirection);
				}
			}
		}
		else
		{
			// Attempt to find a path to the food
			// currentPath = pathFind(game->snakePositions[0].x, game->snakePositions[0].y, game->foodPos.x, game->foodPos.y, convertBoardToSpots());
			// if (!currentPath.empty())
			// 	if (currentPath[0] != "INVALID")
			// 		mode = "PATH";

			if (!safePath.empty())
			{
				game->setDirection(safePath[0]);
				safePath.erase(safePath.begin());
			}
			else
			{
				mode = "PATH";
			}

			/*
			// Ensure that the attempted move will not result in death
			if (game->checkDeathOnUpdate())
			{
				std::string newDirection = "NONE";
				auto prevDir = game->direction;

				// Just try to survive

				game->direction = "UP";
				if (!game->checkDeathOnUpdate())
					newDirection = "UP";

				game->direction = "DOWN";
				if (!game->checkDeathOnUpdate())
					newDirection = "DOWN";

				game->direction = "LEFT";
				if (!game->checkDeathOnUpdate())
					newDirection = "LEFT";

				game->direction = "RIGHT";
				if (!game->checkDeathOnUpdate())
					newDirection = "RIGHT";

				game->setDirection(prevDir);

				if (newDirection != "NONE")
					game->setDirection(newDirection);

				game->snakeR = 255;
				game->snakeG = 255;
				game->snakeB = 255;
			}
			*/
		}

		// Ensure that the entire snake can fit into the section being entered

		auto boardState = convertBoardToSpots(false);

		size_t x = game->snakePositions[0].x;
		size_t y = game->snakePositions[0].y;

		int requiredSize = 2;

		boardState[y][x].f = 1;

		std::vector<std::string> valid = {"UP", "DOWN", "LEFT", "RIGHT"};

		if (mode != "ULTRA_SAFE")
		{
			// Check pointing up
			if (y == 0 || boardState[y - 1][x].wall)
				valid.erase(std::remove(valid.begin(), valid.end(), "UP"), valid.end());

			// Check pointing down
			if (y == game->blocksY - 1 || boardState[y + 1][x].wall)
				valid.erase(std::remove(valid.begin(), valid.end(), "DOWN"), valid.end());

			// Check pointing left
			if (x == 0 || boardState[y][x - 1].wall)
				valid.erase(std::remove(valid.begin(), valid.end(), "LEFT"), valid.end());

			// Check pointing right
			if (x == game->blocksX - 1 || boardState[y][x + 1].wall)
				valid.erase(std::remove(valid.begin(), valid.end(), "RIGHT"), valid.end());

			// Check pointing up
			if (y > 0 && !boardState[y - 1][x].wall)
			{
				std::vector<Spot *> spotsFilled = floodFill(x, y - 1, boardState);

				if (spotsFilled.size() < game->snakePositions.size() + requiredSize)
					valid.erase(std::remove(valid.begin(), valid.end(), "UP"), valid.end());
			}

			// Check pointing down
			if (y < game->blocksY - 1 && !boardState[y + 1][x].wall)
			{
				std::vector<Spot *> spotsFilled = floodFill(x, y + 1, boardState);

				if (spotsFilled.size() < game->snakePositions.size() + requiredSize)
					valid.erase(std::remove(valid.begin(), valid.end(), "DOWN"), valid.end());
			}

			// Check pointing left
			if (x > 0 && !boardState[y][x - 1].wall)
			{
				std::vector<Spot *> spotsFilled = floodFill(x - 1, y, boardState);

				if (spotsFilled.size() < game->snakePositions.size() + requiredSize)
					valid.erase(std::remove(valid.begin(), valid.end(), "LEFT"), valid.end());
			}

			// Check pointing right
			if (x < game->blocksX - 1 && !boardState[y][x + 1].wall)
			{
				std::vector<Spot *> spotsFilled = floodFill(x + 1, y, boardState);

				if (spotsFilled.size() < game->snakePositions.size() + requiredSize)
					valid.erase(std::remove(valid.begin(), valid.end(), "RIGHT"), valid.end());
			}
		}

		if (!valid.empty())
		{
			if (std::find(valid.begin(), valid.end(), game->direction) == valid.end())
			{
				// Pick a new direction from the valid directions
				game->setDirection(valid[0]);
			}
		}
		else
		{
			// The snake is entirely enclosed by walls and/or itself

			if (mode != "ULTRA_SAFE")
			{
				boardState = convertBoardToSpots();

				// Find the piece of the snake closest to the end that has the longest possible path
				mode = "ULTRA_SAFE";
				safePath.clear();
				safePath.resize(0);

				std::vector<std::string> longestPath;
				Spot *finalTarget = nullptr;

				Spot *target = nullptr;

				for (size_t i = game->snakePositions.size() - 1; i > 0; i--)
				{
					// The target position of the snake
					target = &boardState[game->snakePositions[i].y][game->snakePositions[i].x];

					auto tempWall = target->wall;
					auto tempF = target->f;

					target->wall = false;
					target->f = 0;

					auto path = pathFind(game->snakePositions[0].x, game->snakePositions[0].y, target->x, target->y, boardState, true);

					target->wall = tempWall;
					target->f = tempF;

					if (!path.empty() && path[0] != "INVALID")
					{
						// The current path is long enough for the snake to get out safely
						safePath = path;
						finalTarget = target;
						break;
					}

					// if (path.size() > longestPath.size())
					// {
					// 	// The current path is long enough for the snake to get out safely
					// 	longestPath = path;
					// 	finalTarget = target;
					// }
				}

				// safePath = longestPath;

				std::cout << "\n\n\n";
				std::cout << "Target: " << target->x << ", " << target->y << "\n";
				for (const auto &element : safePath)
					std::cout << "Path: " << element << "\n";

				game->setDirection(safePath[0]);
				safePath.erase(safePath.begin());

				// rapid::RapidWarning("Warning", "Snake cannot escape this situation").display();
			}
		}

		if (mode == "PATH")
		{
			game->snakeR = 50;
			game->snakeG = 255;
			game->snakeB = 50;
		}
		else if (mode == "SAFE")
		{
			game->snakeR = 50;
			game->snakeG = 100;
			game->snakeB = 255;
		}
		else if (mode == "ULTRA_SAFE")
		{
			game->snakeR = 255;
			game->snakeG = 170;
			game->snakeB = 170;
		}

		updates++;
	}

	std::vector<std::string> pathFind(size_t startX, size_t startY, size_t findX, size_t findY, std::vector<std::vector<Spot>> &grid, bool longest = false) const
	{
		// auto grid = convertBoardToSpots();

		std::vector<Spot *> openSet;
		std::vector<Spot *> closedSet;
		Spot *start = &grid[startY][startX]; // &grid[game->snakePositions[0].y][game->snakePositions[0].x];
		Spot *end = &grid[findY][findX]; // &grid[game->foodPos.y][game->foodPos.x];

		openSet.push_back(start);

		bool pathFound = false;

		while (!pathFound)
		{
			if (!openSet.empty())
			{
				// Find the closest cell in the open set
				size_t index = 0;
				for (size_t i = 0; i < openSet.size(); i++)
				{
					if (longest)
					{
						if (openSet[i]->f > openSet[index]->f)
							index = i;
					}
					else
					{
						if (openSet[i]->f < openSet[index]->f)
							index = i;
					}
				}

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

							if (longest)
							{
								if (tempG > cell->g)
								{
									// Found shorter path to this cell
									cell->g = tempG;
									newPath = true;
								}
							}
							else
							{
								if (tempG < cell->g)
								{
									// Found shorter path to this cell
									cell->g = tempG;
									newPath = true;
								}
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

	std::vector<Spot *> floodFill(size_t x, size_t y, std::vector<std::vector<Spot>> tempBoard)
	{
		// auto tempBoard = convertBoardToSpots(false);

		Spot *current = &tempBoard[y][x];

		current->addNeighborsConditional(tempBoard);
		current->f = 1;

		std::vector<Spot *> cellsFilled;

		floodFillRecurse(current, tempBoard, &cellsFilled);

		return cellsFilled;
	}

	void floodFillRecurse(Spot *cell, std::vector<std::vector<Spot>> &grid, std::vector<Spot *> *cellsFilled)
	{
		for (auto &cellNeighbor : cell->neighbors)
		{
			if (cellNeighbor->f != 1)
			{
				cellNeighbor->addNeighborsConditional(grid);
				cellNeighbor->f = 1;
				cellsFilled->emplace_back(cellNeighbor);

				floodFillRecurse(cellNeighbor, grid, cellsFilled);
			}
		}
	}

	std::vector<std::vector<Spot>> convertBoardToSpots(bool addNeighbors = true) const
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

		if (addNeighbors)
		{
			for (size_t i = 0; i < game->blocksY; i++)
			{
				for (size_t j = 0; j < game->blocksX; j++)
				{
					result[i][j].addNeighbors(result);
				}
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
