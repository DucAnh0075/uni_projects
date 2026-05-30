#include "models.h"

struct Keybind {
  char key;
  const char *description;
};

const struct Keybind *getKeybinds(enum KeybindsMode mode);
