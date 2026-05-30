#ifndef MODELS_H
#define MODELS_H

#include <stdbool.h>

struct Size {
  int height;
  int width;
};

#define MODE_COUNT 5
enum KeybindsMode {
  HIDDEN_MODE,
  MODE_MENU,
  NORMAL_MODE,
  EDIT_MODE,
  TRADE_MODE,
};

enum GameMode { RELEASE, DEBUG };

enum State {
  LOBBY,
  PLAYING,
  END,
};

enum Color {
  BLACK = 0,
  WHITE = 15,
  GREY = 8,
  BROWN = 94,
  LIGHTBLUE = 39,
  PINK = 207,
  ORANGE = 202,
  RED = 196,
  YELLOW = 226,
  GREEN = 46,
  BLUE = 21,
};

enum FieldType {
  STREET,
  PRISON,
  START,
  GO_TO_PRISON,
  BLACK_JACK,
  FREE_PARKING,
  STATION,
  GPT,
  SEMESTER,
};

struct Field;

struct Position {
  int x;
  int y;
};

struct Player {
  char *name;
  int money;
  struct Player *nextPlayer;
  enum Color color;
  bool inPrison;
  struct Field *currentField;
  int rollCountInPrison;
};

struct Field {
  char *name;
  int price;
  int groupID;
  unsigned buildingPrice;
  unsigned rent[6];
  unsigned rentIndex;
  struct Player *owner;
  enum Color color;
  enum FieldType fieldType;
  struct Field *nextField;
  struct Position *gridPosition;
};

struct GameState {
  struct Player *players;
  struct Field *fields;
  struct Player *currentPlayer;
  struct Position *gridPositionOnBoard;
  struct Position *realPosition;
  enum State state;
  char *bottomBoxText;
  enum KeybindsMode keybindsMode;
  bool showMiniMap;
};

struct Camera {
  struct Field *currentlyViewedField;
  struct Position currentPosition;
};

/*enum cardColor{
    HEARTS,
    SPADES,
    CLUBS,
    DIAMONDS,
};

enum cardSuits{
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE,
};

struct Cards {

};*/

#endif
