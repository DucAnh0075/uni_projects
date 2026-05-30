#include "keybinds.h"

const struct Keybind keybinds[5][4] = {
    [HIDDEN_MODE] = {},
    [MODE_MENU] =
        {
            {'y', "Finish turn"},
            {'n', "Normal mode"},
            {'e', "Edit mode"},
            {'t', "Trade mode"},
        },
    [NORMAL_MODE] =
        {
            {'a', "Move forward"},
            {'d', "Move backward"},
        },
    [EDIT_MODE] =
        {
            {'a', "Move next"},
            {'d', "Move previous"},
            {'b', "Build"},
            {'d', "Destroy"},
        },
    [TRADE_MODE] =
        {
            {'a', "Move next"},
            {'d', "Move previous"},
            {'t', "Trade"},
        },
};

// Function that returns the keybinds for a given action
const struct Keybind *getKeybinds(enum KeybindsMode mode) {
  return keybinds[mode];
}
