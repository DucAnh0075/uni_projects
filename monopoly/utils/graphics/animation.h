#ifndef ANIMATION_H
#define ANIMATION_H

#include "../../config.h"
#include "../gameState.h"
#include "engine.h"
#include <math.h>
#include <stdio.h>

float lerp(float, float, float);

float easeInQuad(float);

float easeInOutQuad(float);

int animateToCenter(struct Position positionToCenter,
                    struct Position *updatePosition, float deltaTime,
                    struct Position shiftCenter);

void resetAnimation();

float getAnimationPercentage();

float setAnimationPercentage(float percentage);

void goToField(struct Position gridPosition);

#endif
