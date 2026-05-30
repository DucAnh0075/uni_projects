#ifndef ENGINE_H
#define ENGINE_H

#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>

#include "../../config.h"
#include "../../models/keybinds.h"
#include "../../models/models.h"
#include "../../utils/dice.h"
#include "../../utils/fileReader.h"
#include "../../utils/gameState.h"

#define FIELD_WIDTH 27
#define FIELD_HEIGHT 12

static int originalFlags;

struct Size getScreenSize();

void drawPixel(struct Position, enum Color, enum Color, wchar_t);

void drawText(struct Position, enum Color, const char *);

void drawField(struct Field *, struct Position);

void drawCursor(enum Color);

void setBottomBoxText(char *);

void drawBottomCenteredBoxText();

void drawRectangle(struct Position topLeft, struct Position bottomRight,
                   bool xCorners, enum Color fgColor);

void clearScreen();

int startEngine();

void stopEngine();

void setCanonMode(bool);

void setEchoMode(bool);

void renderScreen();

bool isRectangleCompletelyHidden(struct Position, struct Position);

void resetBlockingMode(int fd, int originalFlags);

void setNonBlockingMode(int fd);

int loadArtwork();

void drawArtwork(const char *content, struct Position topLeftPosition,
                 enum Color color);

void renderPlayerStats();

void drawMiniMap();

void throwDices(int, int);

void drawDice(int, struct Position);

void drawKeybinds();

struct Position getCenterOfField(struct Position);

struct Position getCenterOfScreen();

struct Position getTopLeftCornerOfFieldByCenter(struct Position);

struct Position getRealTopLeftCorner(struct Position position);

struct Position getRealBottomRightCorner(struct Position position);

struct Position shiftPosition(struct Position position, struct Position shift);

struct Position correctWidthHeightRatio(struct Position position);

#endif
