#ifndef FIELDS_H
#define FIELDS_H
#include "models.h"

struct Field *getBoardFields();

struct Field *createField(char *name, int price, int groupID, int buildingPrice,
                          int rent[], int rentIndex, enum Color color,
                          enum FieldType fieldType, struct Field *nextField,
                          struct Position gridPositon);
#endif
