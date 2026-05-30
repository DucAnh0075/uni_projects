#include "player.h"
#include "../models/models.h"
#include "gameState.h"
#include "graphics/animation.h"
#include "graphics/engine.h"

void enableNonCanonicalMode() {
  struct termios t;
  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag &= ~(ICANON | ECHO);
  t.c_cc[VMIN] = 1;
  t.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void resetCanonicalMode() {
  struct termios t;
  tcgetattr(STDIN_FILENO, &t);
  t.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

unsigned amountPlayer() {
  unsigned amountPlayer = 0;
  while (true) {
    logger("Enter number of players (2-4). Press enter to confirm: \n");
    sprintf(gameState.bottomBoxText,
            "Enter number of players (2-4). Press enter to confirm: \n");
    renderScreen();
    scanf("%u", &amountPlayer);
    if (amountPlayer >= 2 && amountPlayer <= 4)
      break;
    logger("give a valid number!");
    sprintf(gameState.bottomBoxText,
            "give a valid number! Press enter to try again: \n");
    renderScreen();
    sleep(1);
    while (getchar() != '\n')
      ;
  }
  return amountPlayer;
}

struct Player *createPlayers(unsigned playerCount) {
  struct Player *firstPlayer = NULL;
  struct Player *prevPlayer = NULL;
  enum Color colors[] = {RED, BLUE, GREEN, YELLOW};

  while (getchar() != '\n')
    ;

  for (int i = 0; i < playerCount; i++) {
    struct Player *player = malloc(sizeof(struct Player));
    if (player == NULL)
      return NULL;

    player->name = malloc(MAX_NAME_LENGTH * sizeof(char));
    if (player->name == NULL) {
      free(player);
      return NULL;
    }

    while (1) {
      sprintf(gameState.bottomBoxText, "Enter name for Player %d: ", i + 1);
      renderScreen();

      if (fgets(player->name, MAX_NAME_LENGTH, stdin) != NULL) {
        // Remove newline
        player->name[strcspn(player->name, "\n")] = 0;

        // Check input length
        if (strlen(player->name) < MAX_NAME_LENGTH - 1) {
          break; // Valid name, exit retry loop
        }

        sprintf(gameState.bottomBoxText,
                "Name too long. Max %d characters. Try again.",
                MAX_NAME_LENGTH - 1);
        renderScreen();
        while (getchar() != '\n')
          ;
      }
    }

    player->color = colors[i];
    player->money = STARTING_MONEY;
    player->inPrison = false;
    player->currentField = NULL;
    player->nextPlayer = NULL;
    player->rollCountInPrison = 0;

    if (firstPlayer == NULL) {
      firstPlayer = player;
    } else {
      prevPlayer->nextPlayer = player;
    }
    prevPlayer = player;
  }

  if (prevPlayer != NULL) {
    prevPlayer->nextPlayer = firstPlayer;
  }

  return firstPlayer;
}

void gameLoop() {
  char input;
  while (true) {
    playerTurn(gameState.currentPlayer);

    if (gameState.state == END) {
      break;
    }
    gameState.currentPlayer = gameState.currentPlayer->nextPlayer;
    if (gameState.currentPlayer->nextPlayer == gameState.currentPlayer) {
      sprintf(gameState.bottomBoxText, "The player %s won!!!!",
              gameState.currentPlayer->name);
      renderScreen();
      while (getchar() != '\n')
        ;
      break;
    }
  }
}

void movePlayer(struct Player *player, int steps) {
  for (int i = 0; i < steps; i++) {
    player->currentField = player->currentField->nextField;
    if (strcmp(gameState.currentPlayer->currentField->name, "LOS") == 0) {
      sprintf(gameState.bottomBoxText, "You walked over GO. Receive 200$: ");
      renderScreen();
      player->money += 200;
    }
  }
  goToField(*player->currentField->gridPosition);
}

// fctPRISON
void goToPrison(struct Player *player) {
  while (strcmp(gameState.currentPlayer->currentField->name, "Gefängnis") !=
         0) {
    player->currentField = player->currentField->nextField;
  }
  goToField(*player->currentField->gridPosition);
}

void resetPrisonStats() {
  gameState.currentPlayer->inPrison = false;
  gameState.currentPlayer->rollCountInPrison = 0;
  sprintf(gameState.bottomBoxText, "you are free! lucky blud -_-");
  renderScreen();
  sleep(1);
}

void checkIfPrison(struct Player *currentPlayer) {
  if (currentPlayer->inPrison) {
    logger("do you want to buyout of prison? y/n");
    sprintf(gameState.bottomBoxText,
            "do you want to buyout of prison for 300$? y/n");
    renderScreen();
    char input;
    scanf("%c", &input);
    if (input == 'n' || input == 'N') {
      while (getchar() != '\n')
        ;
      currentPlayer->rollCountInPrison++;
      logger("gamble to escape");
      sprintf(gameState.bottomBoxText, "gamble to escape  (attempt %d/3)",
              currentPlayer->rollCountInPrison);
      renderScreen();
      while (getchar() != '\n')
        ;

      int diceValue1 = rollDice();
      int diceValue2 = rollDice();
      int totalMove = diceValue1 + diceValue2;
      throwDices(diceValue1, diceValue2);
      sprintf(gameState.bottomBoxText, "You rolled: %d, %d\n", diceValue1,
              diceValue2);
      renderScreen();
      sleep(1);
      bool escapePrison = is_pash(diceValue1, diceValue2);
      if (escapePrison) {
        resetPrisonStats();
      } else if (currentPlayer->rollCountInPrison >= 3) {
        resetPrisonStats();
        return;
      } else {
        sprintf(gameState.bottomBoxText, "No doubles - turn ends!");
        renderScreen();
        sleep(1);
        return;
      }
    } else {
      while (getchar() != '\n')
        ;
      logger("weak mentality -_-, you got charged 300$");
      sprintf(gameState.bottomBoxText,
              "weak mentality -_-, you got charged 300$");
      currentPlayer->money -= 300;
      renderScreen();
      sleep(1);
      resetPrisonStats();
    }
  }
}

// fctTURN
void playerTurn(struct Player *currentPlayer) {
  goToField(*currentPlayer->currentField->gridPosition);
  sprintf(gameState.bottomBoxText, "\n=== %s's turn ===\n",
          currentPlayer->name);
  renderScreen();
  sleep(1);
  checkIfPrison(currentPlayer);
  int countPash = 0;
  bool canRollAgain;

  do {
    logger("\nPress Enter to roll...");
    sprintf(gameState.bottomBoxText, "Press Enter to roll...");
    renderScreen();
    while (getchar() != '\n')
      ;

    int diceValue1 = rollDice();
    int diceValue2 = rollDice();
    int totalMove = diceValue1 + diceValue2;
    throwDices(diceValue1, diceValue2);

    sprintf(gameState.bottomBoxText, "You rolled: %d, %d (Total: %d)\n",
            diceValue1, diceValue2, totalMove);
    renderScreen();

    canRollAgain = is_pash(diceValue1, diceValue2);
    if (canRollAgain) {
      countPash++;
      if (countPash == 3) {
        logger("Three doubles in a row! Going to jail!");
        sprintf(gameState.bottomBoxText,
                "Three doubles in a row! Going to jail!");
        renderScreen();
        currentPlayer->inPrison = true;
        goToPrison(currentPlayer);
        break;
      }
      if (currentPlayer->inPrison)
        return;
      sprintf(gameState.bottomBoxText,
              "You rolled doubles! Number of doubles: %d\n", countPash);
      renderScreen();
    }

    movePlayer(currentPlayer, totalMove);
    char message[100];
    sprintf(message, "New position: %s\n", currentPlayer->currentField->name);
    logger(message);

    // Field events
    enum FieldType currentType = currentPlayer->currentField->fieldType;
    char input;

    switch (currentType) {
    case STREET:
      if (currentPlayer->currentField->owner != NULL) {
        payRent(currentPlayer);
      } else {
        buyStreet(currentPlayer);
      }
      break;
    default:
      break;
    case GO_TO_PRISON:
      currentPlayer->inPrison = true;
      goToPrison(currentPlayer);
      break;
    case STATION:
      if (currentPlayer->currentField->owner != NULL) {
        payRent(currentPlayer);
      } else {
        buyStreet(currentPlayer);
        updateBahnof(currentPlayer, currentPlayer->currentField);
      }
      break;
    case SEMESTER:
      currentPlayer->money -=
          currentPlayer->currentField
              ->rent[currentPlayer->currentField->rentIndex];
      sprintf(gameState.bottomBoxText, "The new semester starts and the uni "
                                       "rips away your soul and money: -333$.");
      renderScreen();
      sleep(2);
      gameOver(currentPlayer);
      break;
    case GPT:
      currentPlayer->money -=
          currentPlayer->currentField
              ->rent[currentPlayer->currentField->rentIndex];
      sprintf(gameState.bottomBoxText, "At least it's worth it: -222$");
      renderScreen();
      sleep(2);
      gameOver(currentPlayer);
      break;
    }

    if (gameState.state == END) {
      break;
    }

    if (!canRollAgain) {
      logger("Next player's turn? (y/n): ");
      sprintf(gameState.bottomBoxText, "Next player's turn? (y/n): ");
      gameState.keybindsMode = MODE_MENU;
      renderScreen();
      scanf(" %c", &input);
      // if input is of type number
      if (input == 'y' || input == 'Y') {
        while (getchar() != '\n')
          ;
        break;
      } else {
        while (getchar() != '\n')
          ;
        gameState.keybindsMode = EDIT_MODE;
        renderScreen();
        navigateFields(currentPlayer);
      }
    }
  } while (canRollAgain && countPash < 3);
}

// fctACTIONS
void buyHotel(struct Field *buildableField) {
  buildableField->rentIndex++;
  gameState.currentPlayer->money -= buildableField->buildingPrice;
  sprintf(gameState.bottomBoxText,
          "You now have %d buildings and your current money is %d. Press enter "
          "to confirm.",
          buildableField->rentIndex, gameState.currentPlayer->money);
  renderScreen();
  while (getchar() != '\n')
    ;
}

void updateBahnof(struct Player *currentPlayer, struct Field *currentField) {
  if (!currentPlayer || !currentField) {
    return;
  }
  struct Field *startField = currentField;
  do {
    currentField = currentField->nextField;
    if (!currentField) {
      return;
    }
    if (currentField->owner == currentPlayer &&
        currentField->fieldType == STATION) {
      currentField->rentIndex += 1;
    }
  } while (currentField != startField);
}

void buyStreet(struct Player *currentPlayer) {
  char input;
  logger("Do you want to buy the house?: y/n");
  sprintf(gameState.bottomBoxText, "Do you want to buy the house for $%d?: y/n",
          currentPlayer->currentField->price);
  renderScreen();
  scanf(" %c", &input);
  while (getchar() != '\n')
    ;
  // if input is of type number
  if (input == 'y' || input == 'Y') {
    if (currentPlayer->money >= currentPlayer->currentField->price) {
      currentPlayer->money -= currentPlayer->currentField->price;
      currentPlayer->currentField->owner = currentPlayer;
      renderScreen();
    } else {
      sprintf(gameState.bottomBoxText, "too broke blud :(");
      renderScreen();
      sleep(1);
    }
  }
}

void removePlayerFromList(struct Player *playerToRemove) {
  struct Player *current = playerToRemove;
  struct Player *prev = NULL;

  // Find previous player
  do {
    if (current->nextPlayer == playerToRemove) {
      prev = current;
      break;
    }
    current = current->nextPlayer;
  } while (current != playerToRemove);

  // Adjust linked list pointers
  if (prev != playerToRemove->nextPlayer) {
    prev->nextPlayer = playerToRemove->nextPlayer;
  } else {
    gameState.state = END;
  }
}

void gameOver(struct Player *currentPlayer) {
  if (currentPlayer->money <
          currentPlayer->currentField
              ->rent[currentPlayer->currentField->rentIndex] ||
      currentPlayer->money < 0) {
    sprintf(gameState.bottomBoxText,
            "You're broke and couldn't pay the semester fees. You've been "
            "exmatrikuliert!");
    renderScreen();
    sleep(2);
    currentPlayer->currentField->owner->money += currentPlayer->money;
    removePlayerFromList(currentPlayer);
    goToField(*currentPlayer->currentField->gridPosition);
    //gameState.state = END;
  }
}

void payRent(struct Player *currentPlayer) {
  while (getchar() != '\n')
    ;
  if (currentPlayer->currentField->owner != currentPlayer) {
    unsigned rentAmount = currentPlayer->currentField
                              ->rent[currentPlayer->currentField->rentIndex];
    sprintf(gameState.bottomBoxText,
            "You have to pay $%d rent to %s. Press enter to proceed",
            rentAmount, currentPlayer->currentField->owner->name);
    renderScreen();
    while (getchar() != '\n')
      ;
    gameOver(currentPlayer);
    currentPlayer->money -= rentAmount;
    currentPlayer->currentField->owner->money += rentAmount;
    logger("Paid rent");
  }
}
void playerTrade(struct Field *currentlyViewed) {
  struct Player *currentTarget = gameState.currentPlayer->nextPlayer;
  char input;
  int addMoney = 0;
  bool validInput = false;
  while (true) {
    sprintf(gameState.bottomBoxText,
            "Trade with %s? Press Enter to confirm, n for next player, or q to "
            "quit.",
            currentTarget->name);
    renderScreen();
    input = getchar();
    while (getchar() != '\n')
      ;
    switch (input) {
    case 'n':
      currentTarget = currentTarget->nextPlayer;
      if (currentTarget == gameState.currentPlayer) {
        currentTarget = currentTarget->nextPlayer;
      }
      break;
    case '\n':
      sprintf(gameState.bottomBoxText, "Starting trade with %s.",
              currentTarget->name);
      renderScreen();
      sleep(2);
      sprintf(gameState.bottomBoxText, "%s Add money to offer",
              gameState.currentPlayer->name);
      renderScreen();
      sleep(2);

      while (!validInput) {
        sprintf(gameState.bottomBoxText, "Enter the amount you want to offer:");
        renderScreen();

        if (scanf("%d", &addMoney) != 1) {
          sprintf(gameState.bottomBoxText,
                  "Invalid input. Please enter a valid number:");
          renderScreen();
          while (getchar() != '\n')
            ;
        } else if (addMoney > gameState.currentPlayer->money) {
          sprintf(gameState.bottomBoxText,
                  "You don't have that much money! Enter a smaller amount:");
          renderScreen();
          while (getchar() != '\n')
            ;
        } else {
          validInput = true;
          while (getchar() != '\n')
            ;
        }
      }

      sprintf(gameState.bottomBoxText, "You offered %d$", addMoney);
      renderScreen();
      while (getchar() != '\n')
        ;
      return;
    case 'q':
      sprintf(gameState.bottomBoxText, "Trading cancelled.");
      renderScreen();
      return;
    default:
      sprintf(gameState.bottomBoxText,
              "Invalid input. Press n for next player, Enter to confirm, or q "
              "to quit.");
      renderScreen();
      break;
    }
  }
}

// fctCAM
void navigateFields(struct Player *currentPlayer) {
  enableNonCanonicalMode();
  struct Camera camera;
  camera.currentlyViewedField = currentPlayer->currentField;
  camera.currentPosition = *currentPlayer->currentField->gridPosition;

  char input;
  bool exitMode = false;
  struct Field *startField = camera.currentlyViewedField;

  tcflush(STDIN_FILENO, TCIFLUSH);

  while (!exitMode) {
    bool canBuildHere =
        camera.currentlyViewedField->fieldType == STREET &&
        camera.currentlyViewedField->owner == currentPlayer &&
        ownsCompleteSet(currentPlayer, camera.currentlyViewedField);

    if (canBuildHere) {
      sprintf(gameState.bottomBoxText,
              "%s -Press 'b' to build house (Cost: %d)",
              camera.currentlyViewedField->name,
              camera.currentlyViewedField->buildingPrice);
    } else {
      sprintf(gameState.bottomBoxText, "%s", camera.currentlyViewedField->name);
    }
    renderScreen();
    usleep(500);
    tcflush(STDIN_FILENO, TCIFLUSH);
    input = getchar();
    struct Field *nextField = NULL;

    switch (input) {
    case 'a':
      if (camera.currentlyViewedField->nextField != NULL) {
        nextField = camera.currentlyViewedField->nextField;
        camera.currentlyViewedField = nextField;
        goToField(*nextField->gridPosition);
      }
      break;

    case 'd': {
      struct Field *temp = startField;
      struct Field *prev = NULL;
      bool found = false;

      while (temp != NULL && !found) {
        if (temp->nextField == camera.currentlyViewedField) {
          prev = temp;
          found = true;
          break;
        }
        temp = temp->nextField;

        if (temp == startField)
          break;
      }
      if (!found) {
        temp = startField;
        while (temp->nextField != NULL) {
          temp = temp->nextField;
        }
        prev = temp;
      }
      if (prev != NULL) {
        camera.currentlyViewedField = prev;
        goToField(*prev->gridPosition);
      }
      break;
    }

    case 'b':
      if (canBuildHere) {
        resetCanonicalMode();
        tcflush(STDIN_FILENO, TCIFLUSH);

        char buildInput;
        sprintf(gameState.bottomBoxText,
                "Do you want to buy a house: Cost %d?: y/n",
                camera.currentlyViewedField->buildingPrice);
        renderScreen();

        do {
          scanf(" %c", &buildInput);
          while (getchar() != '\n')
            ;
        } while (buildInput != 'y' && buildInput != 'Y' && buildInput != 'n' &&
                 buildInput != 'N');

        if (buildInput == 'y' || buildInput == 'Y') {
          if (camera.currentlyViewedField->rentIndex > 4) {
            sprintf(gameState.bottomBoxText,
                    "You already have a hotel. Can't build anymore. Press "
                    "enter to continue.");
            renderScreen();
            while (getchar() != '\n')
              ;
          } else {
            buyHotel(camera.currentlyViewedField);
          }
        }
        enableNonCanonicalMode();
        tcflush(STDIN_FILENO, TCIFLUSH);
      }

      break;
    case 'q':
      exitMode = true;
      goToField(*startField->gridPosition);
      break;
    case 't':
      playerTrade(camera.currentlyViewedField);
      break;
    }
  }
  resetCanonicalMode();
}

bool ownsCompleteSet(struct Player *player, struct Field *field) {
  if (field->fieldType != STREET || player == NULL)
    return false;
  int targetGroupID = field->groupID;
  struct Field *current = field;
  int groupFieldCount = 0;
  int playerOwnedCount = 0;
  do {
    // Count if its a street in our target group
    if (current->fieldType == STREET && current->groupID == targetGroupID) {
      groupFieldCount++;
      if (current->owner == player) {
        playerOwnedCount++;
      }
    }
    current = current->nextField;
  } while (current != NULL && current != field);
  sprintf(gameState.bottomBoxText, "Group: %d, Total: %d, Owned: %d",
          targetGroupID, groupFieldCount, playerOwnedCount);
  renderScreen();
  usleep(1000000);
  return groupFieldCount > 0 && groupFieldCount == playerOwnedCount;
}
