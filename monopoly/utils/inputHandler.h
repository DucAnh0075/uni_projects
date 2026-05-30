#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <stdio.h>

enum InputKey {
  ARROW_UP,
  ARROW_DOWN,
  ARROW_LEFT,
  ARROW_RIGHT,
  KEY_UNKNOWN,
  NO_INPUT
};

enum InputKey readInput();

#endif
