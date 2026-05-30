#ifndef CONFIG_H
#define CONFIG_H

#include "models/models.h"

#define FRAME_RATE 60
#define ANIMATION_DURATION 0.5
#define TESTING_ENABLED false

extern enum GameMode gameMode;
extern bool loggingEnabled;

void setLogging(bool enabled);

void setGameMode(enum GameMode mode);

enum GameMode getGameMode();

bool isLoggingEnabled();

#endif
