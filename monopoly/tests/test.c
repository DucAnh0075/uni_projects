#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../models/fields.h"
#include "../models/models.h"
#include "../utils/dice.h"
#include "../utils/gameState.h"
#include "../utils/graphics/engine.h"
#include "../utils/player.h"

int main() {
  srand(0);

  struct Field *startField = getBoardFields();
  struct Position boardPosition = {5, 7};

  gameState = (struct GameState){.fields = startField,
                                 .gridPositionOnBoard = &boardPosition,
                                 .realPosition = NULL,
                                 .players = NULL,
                                 .currentPlayer = NULL,
                                 .state = LOBBY,
                                 .bottomBoxText = malloc(200 * sizeof(char)),
                                 .keybindsMode = HIDDEN_MODE,
                                 .showMiniMap = true};

  // set test data
  gameState.fields->nextField->owner = NULL;
  gameState.fields->nextField->rentIndex = 1;

  struct Position realBoardPosition =
      getCenterOfField(*gameState.gridPositionOnBoard);
  gameState.realPosition = &realBoardPosition;

  // run tests
  int amountPlayers = amountPlayer();
  printf("%d\n", amountPlayers);

  struct Player *players = createPlayers(amountPlayers);
  printf("%s\n", players[0].name);

  gameState.currentPlayer = players;
  gameState.currentPlayer->currentField = gameState.fields;

  int amountOfSteps = 0;
  scanf("%d", &amountOfSteps);
  movePlayer(gameState.currentPlayer, amountOfSteps);
  printf("%s\n", gameState.currentPlayer->currentField->name);

  return 0;
}
