#include "engine.h"

static bool isGameGreyedOut = false;

void drawPixel(struct Position position, enum Color fgColor, enum Color bgColor,
               wchar_t c) {
  if (position.y < 1 || position.x < 1 || position.y > getScreenSize().height ||
      position.x > getScreenSize().width) {
    return;
  }

  if (isGameGreyedOut) {
    fgColor = GREY;
    bgColor = BLACK;
  }

  if (bgColor == BLACK) {
    wprintf(L"\033[%d;%dH\033[38;5;%dm%lc\033[0m", position.y, position.x,
            fgColor, c);
  } else {
    wprintf(L"\033[%d;%dH\033[38;5;%dm\033[48;5;%dm%lc\033[0m", position.y,
            position.x, fgColor, bgColor, c);
  }

  fflush(stdout);
}

void drawText(struct Position position, enum Color color, const char *text) {
  if (isGameGreyedOut) {
    color = GREY;
  }

  if (position.y < 1 || position.y > getScreenSize().height) {
    return;
  }

  // check if text overflows the screen
  if (position.x + strlen(text) > getScreenSize().width) {
    // only draw the part that fits
    int overflow = position.x + strlen(text) - getScreenSize().width - 1;

    if (overflow > strlen(text)) {
      return;
    }

    char *newText = malloc(strlen(text) - overflow + 1);
    strncpy(newText, text, strlen(text) - overflow);
    newText[strlen(text) - overflow] = '\0';
    text = newText;
  }

  // check if text is partially hidden (to the left)
  if (position.x < 1) {
    // only draw the part that fits
    int overflow = 1 - position.x;

    if (overflow > strlen(text)) {
      return;
    }

    text += overflow;
    position.x = 1;
  }

  wprintf(L"\033[%d;%dH\033[38;5;%dm%s\033[0m", position.y, position.x, color,
          text);
  fflush(stdout);
}

void drawBottomCenteredBoxText() {
  struct Size screenSize = getScreenSize();
  struct Position position = (struct Position){
      screenSize.width / 2 - strlen(gameState.bottomBoxText) / 2,
      screenSize.height - 1};

  drawRectangle(
      (struct Position){position.x - 1, position.y - 2},
      (struct Position){position.x + strlen(gameState.bottomBoxText) + 2,
                        position.y},
      false, WHITE);

  for (int i = 0; i < strlen(gameState.bottomBoxText) + 2; i++) {
    if (i > 0 && i < strlen(gameState.bottomBoxText) + 1) {
      drawPixel((struct Position){position.x + i, position.y - 1}, WHITE, BLACK,
                gameState.bottomBoxText[i - 1]);
    }
  }
}

void drawRectangle(struct Position topLeft, struct Position bottomRight,
                   bool xCorners, enum Color fgColor) {
  for (int x = topLeft.x; x <= bottomRight.x; x++) {
    drawPixel((struct Position){x, topLeft.y}, fgColor, BLACK, L'─');
    drawPixel((struct Position){x, bottomRight.y}, fgColor, BLACK, L'─');
  }

  for (int y = topLeft.y; y <= bottomRight.y; y++) {
    drawPixel((struct Position){topLeft.x, y}, fgColor, BLACK, L'│');
    drawPixel((struct Position){bottomRight.x, y}, fgColor, BLACK, L'│');
  }

  if (xCorners) {
    drawPixel(topLeft, fgColor, BLACK, L'┼');
    drawPixel((struct Position){bottomRight.x, topLeft.y}, fgColor, BLACK,
              L'┼');
    drawPixel(bottomRight, fgColor, BLACK, L'┼');
    drawPixel((struct Position){topLeft.x, bottomRight.y}, fgColor, BLACK,
              L'┼');
  } else {
    drawPixel(topLeft, fgColor, BLACK, L'┌');
    drawPixel((struct Position){bottomRight.x, topLeft.y}, fgColor, BLACK,
              L'┐');
    drawPixel(bottomRight, fgColor, BLACK, L'┘');
    drawPixel((struct Position){topLeft.x, bottomRight.y}, fgColor, BLACK,
              L'└');
  }

  // clear screen inside
  for (int y = topLeft.y + 1; y < bottomRight.y; y++) {
    for (int x = topLeft.x + 1; x < bottomRight.x; x++) {
      drawPixel((struct Position){x, y}, WHITE, BLACK, L' ');
    }
  }
}

/* Field design:
 */

void drawField(struct Field *field, struct Position realPosition) {
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      struct Position currentPosition =
          (struct Position){realPosition.x + x, realPosition.y + y};
      if (y == 0 || y == FIELD_HEIGHT - 1) {
        if (x == 0 || x == FIELD_WIDTH - 1) {
          // corner
          drawPixel(currentPosition, WHITE, BLACK, L'┼');
        } else {
          // top or bottom
          drawPixel(currentPosition, WHITE, BLACK, L'─');
        }
      } else {
        if ((x == 0 || x == FIELD_WIDTH - 1) && y != 3) {
          // left or right
          drawPixel(currentPosition, WHITE, BLACK, L'│');
        } else {
          // inside

          if (x > 1 && x < FIELD_WIDTH - 2 && y > 0 && y < 3) {
            /*changeBackgroundColor(currentPosition, field->color);*/
          }

          // x=2, y=1 is the street name
          if (x == 2 && y == 1) {
            drawText(currentPosition, field->color, field->name);
            continue;
          }

          // draw houses
          if (y == 2) {
            if ((x - 2) % 3 == 0 && x >= 2 && x < field->rentIndex * 3 + 2) {
              drawPixel(currentPosition, WHITE, BLACK, L'');
              drawPixel(shiftPosition(currentPosition, (struct Position){1, 0}),
                        WHITE, BLACK, L' ');
            }
          }

          // y=3 is a divider
          if (y == 3) {
            if (x == 0 || x == FIELD_WIDTH - 1) {
              drawPixel(currentPosition, WHITE, BLACK, L'┼');
            } else {
              drawPixel(currentPosition, WHITE, BLACK, L'─');
            }
            continue;
          }

          // x=2, y=5 is the price
          if (x == 2 && y == 4) {
            char price[FIELD_WIDTH - 4];
            if (field->fieldType == STREET || field->fieldType == STATION) {
              if (field->owner == NULL) {
                sprintf(price, "Buy-price: $%d", field->price);
                drawText(currentPosition, WHITE, price);
                continue;
              } else {
                sprintf(price, "Rent: $%d", field->rent[field->rentIndex]);
                drawText(currentPosition, WHITE, price);
                continue;
              }
            } else if (field->fieldType == BLACK_JACK ||
                       field->fieldType == GPT ||
                       field->fieldType == SEMESTER) {
              sprintf(price, "Pay: $%d", field->price);
              drawText(currentPosition, WHITE, price);
            } else if (field->fieldType == START) {
              sprintf(price, "Collect: $%d", field->price);
              drawText(currentPosition, WHITE, price);
            }
          }

          if (x == 2 && y == 5 && field->owner != NULL) {
            char owner[FIELD_WIDTH - 4];
            sprintf(owner, "Owner: %s", field->owner->name);
            drawText(currentPosition, WHITE, owner);
            continue;
          }

          // print current players
          if (x == 2 && y == 6) {
            struct Player *currentPlayer = gameState.players;
            int amountOfPlayersOnThisField = 0;
            // TODO: player index map: (if two players are on the same field,
            // the second player is place below the first player)
            do {
              if (currentPlayer->currentField == field) {
                drawText(shiftPosition(
                             currentPosition,
                             (struct Position){0, amountOfPlayersOnThisField}),
                         currentPlayer->color, currentPlayer->name);
                amountOfPlayersOnThisField++;
              }

              currentPlayer = currentPlayer->nextPlayer;
            } while (currentPlayer != gameState.players);
          }
        }
      }
    }
  }
}

void drawCursor(enum Color color) {
  struct Position realPosition =
      shiftPosition(getCenterOfScreen(),
                    (struct Position){-FIELD_WIDTH / 2, -FIELD_HEIGHT / 2});
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      struct Position currentPosition =
          (struct Position){realPosition.x + x, realPosition.y + y};
      if (x == 0 && y == 0) {
        drawPixel(currentPosition, color, BLACK, L'┌');
      } else if (x == FIELD_WIDTH - 1 && y == 0) {
        drawPixel(currentPosition, color, BLACK, L'┐');
      } else if (x == 0 && y == FIELD_HEIGHT - 1) {
        drawPixel(currentPosition, color, BLACK, L'└');
      } else if (x == FIELD_WIDTH - 1 && y == FIELD_HEIGHT - 1) {
        drawPixel(currentPosition, color, BLACK, L'┘');
      } else if (x == 0 || x == FIELD_WIDTH - 1) {
        drawPixel(currentPosition, color, BLACK, L'│');
      } else if (y == 0 || y == FIELD_HEIGHT - 1) {
        drawPixel(currentPosition, color, BLACK, L'─');
      }
    }
  }
}

void clearScreen() { wprintf(L"\033[2J"); }

int startEngine() {
  const int err = loadArtwork();

  if (err) {
    return err;
  }

  setCanonMode(false);
  setEchoMode(false);

  /*originalFlags = fcntl(STDIN_FILENO, F_GETFL, 0);*/
  /*setNonBlockingMode(STDIN_FILENO);*/

  clearScreen();

  return 0;
}

void stopEngine() {
  setCanonMode(true);
  setEchoMode(true);

  /*resetBlockingMode(STDIN_FILENO, originalFlags);*/

  clearScreen();

  // Reset cursor position
  wprintf(L"\033[0;0H");
}

void setCanonMode(bool on) {
  struct termios terminal;
  tcgetattr(1, &terminal);

  if (on) {
    terminal.c_lflag |= ICANON;
  } else {
    terminal.c_lflag &= ~ICANON;
  }

  tcsetattr(1, TCSANOW, &terminal);
}

void setEchoMode(bool on) {
  struct termios terminal;
  tcgetattr(1, &terminal);

  if (on) {
    terminal.c_lflag |= ECHO;
  } else {
    terminal.c_lflag &= ~ECHO;
  }

  tcsetattr(1, TCSANOW, &terminal);
}

struct Size getScreenSize() {
  struct winsize size;
  ioctl(1, TIOCGWINSZ, &size);

  return (struct Size){size.ws_row, size.ws_col};
}

bool isRectangleCompletelyHidden(struct Position topLeftReal,
                                 struct Position bottomRightReal) {
  struct Size screenSize = getScreenSize();

  return topLeftReal.x > screenSize.width ||
         topLeftReal.y > screenSize.height || bottomRightReal.x < 0 ||
         bottomRightReal.y < 0;
}

struct Position getRealTopLeftCorner(struct Position position) {
  return (struct Position){position.x * (FIELD_WIDTH - 1) + 1,
                           position.y * (FIELD_HEIGHT - 1) + 1};
}

struct Position getRealBottomRightCorner(struct Position position) {
  return (struct Position){position.x * (FIELD_WIDTH - 1) + FIELD_WIDTH - 2,
                           position.y * (FIELD_HEIGHT - 1) + FIELD_HEIGHT - 2};
}

struct Position shiftPosition(struct Position position, struct Position shift) {
  return (struct Position){position.x + shift.x, position.y + shift.y};
}

struct Position correctWidthHeightRatio(struct Position position) {
  return (struct Position){position.x * 2, position.y};
}

static char *artwork;
static struct Size artworkSize;
int loadArtwork() {
  const char *filename = "./media/artwork.txt";
  artwork = readFile(filename);
  if (!artwork) {
    printf("Failed to read file.\n");
    return 1;
  }

  artworkSize = getArtworkSize(artwork);

  return 0;
}

void renderPlayerStats() {
  if (gameState.showMiniMap) {
    isGameGreyedOut = false;
  }
  // draw player stats (top right corner of screen)
  struct Position playerStatsPosition =
      (struct Position){getScreenSize().width - 30, 1};
  char buffer[100];

  struct Player *thisPlayer = gameState.players;
  int playerNum = 1;
  do {
    // clear area under text
    playerStatsPosition.x -= 2;
    for (int i = 0; i < 6; i++) {
      drawText(playerStatsPosition, WHITE, "                                 ");
      drawPixel(playerStatsPosition, WHITE, BLACK, L'│');
      playerStatsPosition.y += 1;
    }
    playerStatsPosition.y -= 6;
    playerStatsPosition.x += 2;

    sprintf(buffer, "Player %d: %s", playerNum, thisPlayer->name);
    drawText(playerStatsPosition,
             gameState.currentPlayer == thisPlayer ? thisPlayer->color : WHITE,
             buffer);
    playerStatsPosition.y += 1;
    sprintf(buffer, "Money: $%d", thisPlayer->money);
    drawText(playerStatsPosition, WHITE, buffer);
    playerStatsPosition.y += 1;
    sprintf(buffer, "Field: %s", thisPlayer->currentField->name);
    drawText(playerStatsPosition, WHITE, buffer);
    playerStatsPosition.y += 1;
    sprintf(buffer, "Price: %d", thisPlayer->currentField->price);
    drawText(playerStatsPosition, WHITE, buffer);
    playerStatsPosition.y += 1;
    sprintf(buffer, "Owned: %s",
            thisPlayer->currentField->owner == thisPlayer ? "yes" : "no");
    drawText(playerStatsPosition, WHITE, buffer);

    playerStatsPosition.y += 2;

    thisPlayer = thisPlayer->nextPlayer;
    playerNum++;
  } while (thisPlayer != gameState.players);

  // clear 2 lines
  playerStatsPosition.x -= 2;
  drawText(playerStatsPosition, WHITE, "                                 ");
  drawPixel(playerStatsPosition, WHITE, BLACK, L'│');
  playerStatsPosition.y += 1;
  drawText(playerStatsPosition, WHITE, "                                 ");
  drawPixel(playerStatsPosition, WHITE, BLACK, L'│');
  playerStatsPosition.x += 2;

  // info text
  drawText(playerStatsPosition, GREY, "Press 'q' to quit the game");
  playerStatsPosition.y += 1;

  // draw closing line
  playerStatsPosition.x -= 2;
  drawPixel(playerStatsPosition, WHITE, BLACK, L'└');
  playerStatsPosition.x += 1;
  for (int i = 0; i < 32; i++) {
    drawPixel(playerStatsPosition, WHITE, BLACK, L'─');
    playerStatsPosition.x += 1;
  }

  if (gameState.showMiniMap) {
    isGameGreyedOut = true;
  }
}

void renderScreen() {
  if (TESTING_ENABLED) {
    return;
  }

  struct Field *startField = gameState.fields;

  struct Position realBoardPosition = *gameState.realPosition;

  // shift the hole board to the field such that it is on the top left corner
  realBoardPosition.x = -1 * realBoardPosition.x;
  realBoardPosition.y = -1 * realBoardPosition.y;

  clearScreen();

  if (gameState.showMiniMap) {
    isGameGreyedOut = true;
  }

  struct Position center = getCenterOfScreen();

  // show artwork
  struct Position artworkCenter =
      (struct Position){-artworkSize.width / 2, -artworkSize.height / 2};
  drawArtwork(
      artwork,
      shiftPosition(shiftPosition(getRealTopLeftCorner((struct Position){5, 5}),
                                  realBoardPosition),
                    artworkCenter),
      WHITE);

  struct Field *currentField = gameState.fields;
  int y = 1;
  do {
    const struct Position realTopLeftCorner = shiftPosition(
        getRealTopLeftCorner(*currentField->gridPosition), realBoardPosition);
    const struct Position realBottomRightCorner =
        shiftPosition(getRealBottomRightCorner(*currentField->gridPosition),
                      realBoardPosition);
    if (!isRectangleCompletelyHidden(realTopLeftCorner,
                                     realBottomRightCorner)) {
      drawField(currentField, realTopLeftCorner);
    }
    currentField = currentField->nextField;
  } while (currentField != startField);

  if (gameState.state == PLAYING) {
    renderPlayerStats();
  }

  if (gameState.state == PLAYING) {
    // render cursor
    drawCursor(PINK);
  }

  drawKeybinds();

  drawBottomCenteredBoxText();

  // draw minimap  in the center of the screen
  if (gameState.showMiniMap) {
    isGameGreyedOut = false;
    drawMiniMap();
    isGameGreyedOut = true;
  }
}

void setNonBlockingMode(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void resetBlockingMode(int fd, int originalFlags) {
  fcntl(fd, F_SETFL, originalFlags);
}

struct Position getCenterOfScreen() {
  struct Size screenSize = getScreenSize();

  return (struct Position){screenSize.width / 2, screenSize.height / 2};
}

struct Position getCenterOfField(struct Position gridPosition) {
  struct Position realPosition = getRealTopLeftCorner(gridPosition);
  realPosition.x += FIELD_WIDTH / 2;
  realPosition.y += FIELD_HEIGHT / 2;

  return realPosition;
}

void drawArtwork(const char *content, struct Position topLeftPosition,
                 enum Color color) {
  if (!content) {
    return;
  }

  int x = topLeftPosition.x;
  int y = topLeftPosition.y;

  for (int i = 0; content[i] != '\0'; ++i) {
    if (content[i] == '\n') {
      y++;
      x = topLeftPosition.x;
    } else {
      drawPixel((struct Position){x, y}, color, BLACK, content[i]);
      x++;
    }
  }
}

void drawMiniMapField(struct Field *field,
                      struct Position boardTopLeftRealPosition, int fieldSize) {
  struct Position topLeft = (struct Position){
      boardTopLeftRealPosition.x + field->gridPosition->x * fieldSize * 2,
      boardTopLeftRealPosition.y + field->gridPosition->y * fieldSize};
  struct Position bottomRight =
      (struct Position){topLeft.x + fieldSize * 2, topLeft.y + fieldSize};

  drawRectangle(topLeft, bottomRight, true, WHITE);

  // draw player color dot
  struct Position playerDot = (struct Position){topLeft.x + 1, topLeft.y + 1};
  struct Player *currentPlayer = gameState.players;
  if (fieldSize > 2) {
    playerDot.y++;
  }

  int amountOfPlayersOnField = 0;
  do {
    if (currentPlayer->currentField == field) {
      drawPixel(playerDot, currentPlayer->color, BLACK, L'●');
      playerDot.x += 1;
      amountOfPlayersOnField++;
    }

    currentPlayer = currentPlayer->nextPlayer;
  } while (currentPlayer != gameState.players);

  if (fieldSize > 2 || !amountOfPlayersOnField) {
    // draw houses in first line
    if (field->fieldType == STREET && field->owner != NULL) {
      // print like this: 3
      int houseCount = field->rentIndex;
      wchar_t wideStr[10];
      swprintf(wideStr, sizeof(wideStr) / sizeof(wchar_t), L"%d", houseCount);

      drawPixel((struct Position){topLeft.x + 1, topLeft.y + 1},
                field->owner->color, BLACK, wideStr[0]);
      drawPixel((struct Position){topLeft.x + 2, topLeft.y + 1},
                field->owner->color, BLACK, L'');
    }
  }
}

void drawMiniMap() {
  // 80% of screen height
  // same width
  // shows buildings, who is where, who owns what, what can be bought
  struct Size screenSize = getScreenSize();
  struct Size miniMapSize =
      (struct Size){(screenSize.width * 0.80) / 4, screenSize.width * 0.80};
  struct Position topLeft = getCenterOfScreen();

  // draw fields
  // 11 fields in a row
  // Rectangle padding: 2px

  // field size is: ((height + 10)-4)/11 | + 10 because each field (except the
  // first one) overlaps by 1px
  int fieldSize = ((miniMapSize.height) + 10 - 4) / 11;

  struct Size actualMiniMapSize =
      (struct Size){fieldSize * 11 + 2, fieldSize * 11 + 2};

  topLeft.x -= actualMiniMapSize.height;
  topLeft.y -= actualMiniMapSize.height / 2;

  struct Position bottomRight =
      (struct Position){topLeft.x + actualMiniMapSize.height * 2,
                        topLeft.y + actualMiniMapSize.height};

  drawRectangle(topLeft, bottomRight, false, WHITE);

  struct Field *startField = gameState.fields;
  struct Field *currentField = gameState.fields;
  do {
    drawMiniMapField(currentField,
                     (struct Position){topLeft.x + 2, topLeft.y + 1},
                     fieldSize);
    currentField = currentField->nextField;
  } while (currentField != startField);
}

void throwDices(int dice1, int dice2) {
  if (dice1 > 6 || dice2 > 6 || dice1 < 1 || dice2 < 1) {
    return;
  }

  struct Size center = getScreenSize();
  struct Position dice1Position =
      (struct Position){center.width / 2 - 8, center.height - 10};
  struct Position dice2Position =
      (struct Position){center.width / 2 + 1, center.height - 10};

  drawRectangle((struct Position){dice1Position.x - 3, dice1Position.y - 2},
                (struct Position){dice1Position.x + 19, dice1Position.y + 4},
                false, WHITE);

  int rollCount = rollDice() + 4;
  for (int i = 0; i < rollCount; i++) {
    drawDice(rollDice(), dice1Position);
    drawDice(rollDice(), dice2Position);
    int sleepTime = (5 - pow(1.2, 7 - i)) * 100000;
    char message[100];
    sprintf(message, "Sleep time: %d", sleepTime);
    usleep(sleepTime);
  }

  drawDice(dice1, dice1Position);
  drawDice(dice2, dice2Position);

  sleep(2);
}

void drawDice(int dice, struct Position topLeftPosition) {
  if (dice > 6 || dice < 1) {
    return;
  }

  int diceMap[6][3][3] = {
      {
          {0, 0, 0},
          {0, 1, 0},
          {0, 0, 0},
      },
      {
          {1, 0, 0},
          {0, 0, 0},
          {0, 0, 1},
      },
      {
          {1, 0, 0},
          {0, 1, 0},
          {0, 0, 1},
      },
      {
          {1, 0, 1},
          {0, 0, 0},
          {1, 0, 1},
      },
      {
          {1, 0, 1},
          {0, 1, 0},
          {1, 0, 1},
      },
      {
          {1, 0, 1},
          {1, 0, 1},
          {1, 0, 1},
      },
  };

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {

      if (diceMap[dice - 1][y][x]) {
        drawPixel((struct Position){topLeftPosition.x + x * 2 + 1,
                                    topLeftPosition.y + y},
                  WHITE, GREY, L'');
      } else {
        drawPixel((struct Position){topLeftPosition.x + x * 2 + 1,
                                    topLeftPosition.y + y},
                  WHITE, GREY, L' ');
      }
      drawPixel((struct Position){topLeftPosition.x + x * 2 + 2,
                                  topLeftPosition.y + y},
                WHITE, GREY, L' ');
    }
  }
}

void drawKeybinds() {
  // draw keybinds map (based on mode)
  if (gameState.keybindsMode == HIDDEN_MODE) {
    return;
  }

  // keybinds map
  const struct Keybind *keybinds = getKeybinds(gameState.keybindsMode);
  int amountOfKeybinds = 0;
  int maxKeybindLength = 0;

  struct Size screenSize = getScreenSize();
  // get amount of keybinds
  while (keybinds[amountOfKeybinds].key != '\0') {
    // get length of description
    unsigned long length = strlen(keybinds[amountOfKeybinds].description);
    if (length > maxKeybindLength) {
      maxKeybindLength = length;
    }

    amountOfKeybinds++;
  }

  struct Position bottemKeybindsPosition = (struct Position){
      screenSize.width - maxKeybindLength - 7, screenSize.height - 1};

  // draw rectangle around
  drawRectangle((struct Position){screenSize.width - maxKeybindLength - 9,
                                  screenSize.height - amountOfKeybinds - 1},
                (struct Position){screenSize.width - 3, screenSize.height},
                false, WHITE);

  // print keybinds
  for (int i = 0; keybinds[i].key != '\0'; i++) {
    char keybind[100];
    sprintf(keybind, "%c: %s", keybinds[i].key, keybinds[i].description);
    drawText(bottemKeybindsPosition, WHITE, keybind);
    bottemKeybindsPosition.y -= 1;
  }
}
