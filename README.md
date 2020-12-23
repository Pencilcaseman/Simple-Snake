# Simple-Snake

A very simple implementation of the game "Snake" that makes use of [Rapid](https://github.com/Pencilcaseman/Rapid)

The idea with this project is that you can write code in the "Bot" class to produce an algorithm that is able to play snake as well as possible, though you could just play the game yourself if you wanted to...

## The Bot

I have written a fairly advanced bot to play the game, which has 3 different modes and each is represented by a color of the snake. Green means that the snake has found a path to the food using the A\* algorithm and is following it. Blue means no path was found, but it is in no danger of crashing, so simply continues moving around. A light-red color means that the snake has detected that it cannot fill the space it is in, and therfore uses a custom version of the A\* algorithm (creatively named F-) that finds the longest possible path bewteen two points.

The exact details of the algorithm are quite complex, though can be worked out fairly easily by reading the comments in the code in the "bot.h" file.

## Rules

1. Don't crash into yourself
2. Don't crash into the edges of the screen
3. Don't crash into any walls
4. Eat as many food blocks as possible

## Controls

**WASD** and **ARROW KEYS**
