#include "fields.h"
#include "models.h"
#include <stdlib.h> // For malloc and free

struct Field *createField(char *name, int price, int groupID, int buildingPrice,
                          int rent[], int rentIndex, enum Color color,
                          enum FieldType fieldType, struct Field *nextField,
                          struct Position gridPosition) {
  struct Field *field = (struct Field *)malloc(sizeof(struct Field));
  if (field == NULL) {
    // Handle allocation failure (e.g., return NULL or exit)
    return NULL;
  }

  field->name = name;
  field->price = price;
  field->groupID = groupID;
  field->buildingPrice = buildingPrice;
  for (int i = 0; i < 6; i++) {
    field->rent[i] = rent[i];
  }
  field->rentIndex = rentIndex;
  field->color = color;
  field->fieldType = fieldType;
  field->nextField = nextField;

  struct Position *allocatedGridPosition =
      (struct Position *)malloc(sizeof(struct Position));
  allocatedGridPosition->x = gridPosition.x;
  allocatedGridPosition->y = gridPosition.y;
  field->gridPosition = allocatedGridPosition;

  return field;
}

struct Field *getBoardFields() {
  int rent[] = {120, 200, 250, 280, 350, 500};

  //.Seite 4
  struct Field *field39 = createField(
      "Audimax II", 400, 9, 200, (int[]){50, 200, 600, 1400, 1700, 2000}, 0,
      BLUE, STREET, NULL, (struct Position){10, 9});
  struct Field *field38 = createField(
      "Semestergebühren", 333, 0, 0, (int[]){333, 333, 333, 333, 333, 333}, 0,
      WHITE, SEMESTER, field39, (struct Position){10, 8});
  struct Field *field37 = createField(
      "Audimax I", 350, 9, 200, (int[]){35, 175, 500, 1100, 1300, 1500}, 0,
      BLUE, STREET, field38, (struct Position){10, 7});
  struct Field *field36 =
      createField("Ereignisfeld", 0, 0, 0, (int[]){0, 0, 0, 0, 0, 0}, 0, WHITE,
                  BLACK_JACK, field37, (struct Position){10, 6});
  struct Field *field35 = createField(
      "Harburg-Rathaus", 200, 2, 0, (int[]){0, 25, 50, 100, 200, 200}, 0, GREY,
      STATION, field36, (struct Position){10, 5});
  struct Field *field34 =
      createField("LuK", 320, 8, 200, (int[]){28, 150, 450, 1000, 1200, 1400},
                  0, GREEN, STREET, field35, (struct Position){10, 4});
  struct Field *field33 =
      createField("Gemeinschaftsfeld", 0, 0, 0, (int[]){0, 0, 0, 0, 0, 0}, 0,
                  WHITE, BLACK_JACK, field34, (struct Position){10, 3});
  struct Field *field32 = createField(
      "Lernzentrum", 300, 8, 200, (int[]){26, 130, 390, 900, 1100, 1275}, 0,
      GREEN, STREET, field33, (struct Position){10, 2});
  struct Field *field31 = createField(
      "Gendertoilette", 300, 8, 200, (int[]){26, 130, 390, 900, 1100, 1275}, 0,
      GREEN, STREET, field32, (struct Position){10, 1});
  // 3.Seite
  struct Field *field30 =
      createField("Geh ins ", 0, 0, 0, (int[]){0, 0, 0, 0, 0, 0}, 0, WHITE,
                  GO_TO_PRISON, field31, (struct Position){10, 0});
  struct Field *field29 = createField(
      "Infothek", 280, 7, 150, (int[]){24, 120, 360, 850, 1025, 1200}, 0,
      YELLOW, STREET, field30, (struct Position){9, 0});
  struct Field *field28 =
      createField("Ententeich", 150, 5, 0, (int[]){0, 0, 0, 0, 0, 0}, 0, GREY,
                  STREET, field29, (struct Position){8, 0}); // Platzhalter
  struct Field *field27 = createField(
      "C-Gebäude", 260, 7, 150, (int[]){22, 110, 330, 800, 975, 1150}, 0,
      YELLOW, STREET, field28, (struct Position){7, 0});
  struct Field *field26 = createField(
      "D-Gebäude", 260, 7, 150, (int[]){22, 110, 330, 800, 975, 1150}, 0,
      YELLOW, STREET, field27, (struct Position){6, 0});
  struct Field *field25 = createField(
      "Kasernenstraße", 200, 2, 0, (int[]){0, 25, 50, 100, 200, 200}, 0, GREY,
      STATION, field26, (struct Position){5, 0}); // Platzhalter
  struct Field *field24 = createField(
      "F-Gebäude", 240, 6, 150, (int[]){20, 100, 300, 750, 925, 1100}, 0, RED,
      STREET, field25, (struct Position){4, 0});
  struct Field *field23 = createField(
      "G-Gebäude", 220, 6, 150, (int[]){18, 90, 250, 700, 875, 1050}, 0, RED,
      STREET, field24, (struct Position){3, 0});
  struct Field *field22 =
      createField("Ereignisfeld", 0, 0, 0, (int[]){0, 0, 0, 0, 0, 0}, 0, WHITE,
                  BLACK_JACK, field23, (struct Position){2, 0});
  struct Field *field21 =
      createField("Bib", 220, 6, 150, (int[]){18, 90, 250, 700, 875, 1050}, 0,
                  RED, STREET, field22, (struct Position){1, 0});
  // 2.Seite
  struct Field *field20 =
      createField("Pokertisch/FreiParken", 0, 0, 0, (int[]){0, 0, 0, 0, 0, 0},
                  0, WHITE, FREE_PARKING, field21, (struct Position){0, 0});
  struct Field *field19 =
      createField("Mensa", 200, 5, 100, (int[]){16, 80, 220, 600, 800, 1000}, 0,
                  ORANGE, STREET, field20, (struct Position){0, 1});
  struct Field *field18 =
      createField("Pizzabar", 180, 5, 100, (int[]){14, 70, 200, 550, 750, 950},
                  0, ORANGE, STREET, field19, (struct Position){0, 2});
  struct Field *field17 =
      createField("Gemeinschaftsfeld", 0, 0, 0, (int[]){0, 0, 0, 0, 0, 0}, 0,
                  WHITE, BLACK_JACK, field18, (struct Position){0, 3});
  struct Field *field16 =
      createField("Cafeteria", 180, 5, 100, (int[]){14, 70, 200, 550, 750, 950},
                  0, ORANGE, STREET, field17, (struct Position){0, 4});
  struct Field *field15 = createField(
      "S-Bahn Heimfeld", 200, 2, 0, (int[]){0, 25, 50, 100, 200, 200}, 0, GREY,
      STATION, field16, (struct Position){0, 5}); // Platzhalter
  struct Field *field14 =
      createField("Hafen", 160, 4, 100, (int[]){12, 60, 180, 500, 700, 900}, 0,
                  PINK, STREET, field15, (struct Position){0, 6});
  struct Field *field13 =
      createField("K-Gebäude", 140, 4, 100, (int[]){10, 50, 150, 450, 625, 750},
                  0, PINK, STREET, field14, (struct Position){0, 7});
  struct Field *field12 =
      createField("Bafög-Amt", 150, 5, 0, (int[]){0, 0, 0, 0, 0, 0}, 0, GREY,
                  STREET, field13, (struct Position){0, 8}); // Platzhalter
  struct Field *field11 =
      createField("L-Gebäude", 140, 4, 100, (int[]){10, 50, 150, 450, 625, 750},
                  0, PINK, STREET, field12, (struct Position){0, 9});
  // 1.Seite
  struct Field *field10 =
      createField("Gefängnis", 0, 0, 0, (int[]){0, 0, 0, 0, 0, 0}, 0, WHITE,
                  PRISON, field11, (struct Position){0, 10});
  struct Field *field9 =
      createField("M-Gebäude", 120, 3, 50, (int[]){8, 40, 100, 300, 450, 600},
                  0, LIGHTBLUE, STREET, field10, (struct Position){1, 10});
  struct Field *field8 =
      createField("N-Gebäude", 100, 3, 50, (int[]){6, 30, 90, 270, 400, 550}, 0,
                  LIGHTBLUE, STREET, field9, (struct Position){2, 10});
  struct Field *field7 =
      createField("Ereignisfeld", 0, 0, 0, (int[]){0, 0, 0, 0, 0, 0}, 0, WHITE,
                  BLACK_JACK, field8, (struct Position){3, 10});
  struct Field *field6 =
      createField("O-Gebäude", 100, 3, 50, (int[]){6, 30, 90, 270, 400, 550}, 0,
                  LIGHTBLUE, STREET, field7, (struct Position){4, 10});
  struct Field *field5 = createField(
      "Eißendorfer Straße", 200, 2, 0, (int[]){0, 25, 50, 100, 200, 200}, 0,
      GREY, STATION, field6, (struct Position){5, 10}); // Platzhalter
  struct Field *field4 = createField(
      "GPT-Subscribtion", 222, 0, 0, (int[]){222, 222, 222, 222, 222, 222}, 0,
      WHITE, GPT, field5, (struct Position){6, 10});
  struct Field *field3 =
      createField("P-Gebäude", 60, 1, 50, (int[]){4, 20, 60, 180, 320, 450}, 0,
                  BROWN, STREET, field4, (struct Position){7, 10});
  struct Field *field2 =
      createField("Gemeinschaftsfeld", 0, 0, 0, (int[]){0, 0, 0, 0, 0, 0}, 0,
                  WHITE, BLACK_JACK, field3, (struct Position){8, 10});
  struct Field *field1 =
      createField("Q-Gebäude", 60, 1, 50, (int[]){2, 10, 30, 90, 160, 250}, 0,
                  BROWN, STREET, field2, (struct Position){9, 10});
  struct Field *startField =
      createField("LOS", 200, 0, 0, (int[]){0, 0, 0, 0, 0, 0}, 0, WHITE, START,
                  field1, (struct Position){10, 10});

  field39->nextField = startField;

  return startField;
}
