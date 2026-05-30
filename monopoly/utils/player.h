#include "../models/models.h"
#include "dice.h"
#include "gameState.h"
#include "logger.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>

#define STARTING_MONEY 1500
#define MAX_NAME_LENGTH 50
#define BOARD_SIZE 40

void enableNonCanonicalMode();

void resetCanonicalMode();

unsigned amountPlayer();

struct Player *createPlayers(unsigned playerCount);

void gameLoop();

void movePlayer(struct Player *player, int steps);

void goToPrison(struct Player *player);

void resetPrisonStats();

void playerTurn(struct Player *currentPlayer);

void buyHotel(struct Field *buildableField);

void buyStreet(struct Player *currentPlayer);

void payRent(struct Player *currentPlayer);

void playerTrade();

void navigateFields(struct Player *currentPlayer);

bool ownsCompleteSet(struct Player *player, struct Field *field);

void playerTrade(struct Field *currentlyViewed);

void updateBahnof(struct Player *currentPlayer, struct Field *currentField);

void gameOver(struct Player *currentPlayer);
