#include "config.h"

enum GameMode gameMode;
bool loggingEnabled;

void setGameMode(enum GameMode mode) { gameMode = mode; }

void setLogging(bool enabled) { loggingEnabled = enabled; }

enum GameMode getGameMode() { return gameMode; }

bool isLoggingEnabled() { return loggingEnabled; }
