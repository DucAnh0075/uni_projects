#include <locale.h>
#include <stdio.h>
#include <unistd.h>

#include "config.h"
#include "models/fields.h"
#include "models/models.h"
#include "utils/dice.h"
#include "utils/gameState.h"
#include "utils/graphics/animation.h"
#include "utils/graphics/engine.h"
#include "utils/player.h"

int main(int argc, char **argv) {
  // default flags
  setGameMode(RELEASE);
  setLogging(false);

  // set game mode (RELEASE, DEBUG)
  // read in flag --debug && --log
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--debug") == 0) {
      setGameMode(DEBUG);
    } else if (strcmp(argv[i], "--log") == 0) {
      setLogging(true);
      logger("Logging enabled.\n");
    }
  }

  setlocale(LC_ALL, "");

  srand((unsigned int)time(NULL));

  if (startEngine()) {
    logger("Failed to start engine.\n");
    return 1;
  }

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
                                 .showMiniMap = false};
  sprintf(gameState.bottomBoxText, "Press any key to start the game.");

  struct Position realBoardPosition =
      getCenterOfField(*gameState.gridPositionOnBoard);
  gameState.realPosition = &realBoardPosition;

  // go to lobby position
  setAnimationPercentage(1);
  animateToCenter(*gameState.realPosition, gameState.realPosition, 0,
                  (struct Position){5, 5});
  renderScreen();
  resetAnimation();

  // get number of players and create them
  unsigned playerCount = amountPlayer();
  struct Player *firstPlayer = createPlayers(playerCount);

  if (firstPlayer == NULL) {
    logger("Failed to create players.");
    stopEngine();
    return 1;
  }

  // set up each players starting field
  struct Player *currentPlayer = firstPlayer;
  do {
    currentPlayer->currentField = startField;
    currentPlayer = currentPlayer->nextPlayer;
  } while (currentPlayer != firstPlayer);

  gameState.players = firstPlayer;
  gameState.currentPlayer = firstPlayer;

  // set test data
  gameState.fields->nextField->owner = gameState.players;
  gameState.fields->nextField->rentIndex = 1;
  gameState.fields->nextField->nextField->nextField->owner = gameState.players;

  // wait one second
  // getchar();

  sprintf(gameState.bottomBoxText, "Ready, set, go!");

  gameState.gridPositionOnBoard = startField->gridPosition;

  logger("Game started.\n");

  goToField(*gameState.gridPositionOnBoard);

  gameState.state = PLAYING;

  gameLoop();

  stopEngine();

  return 0;
}
