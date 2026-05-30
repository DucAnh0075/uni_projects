#include "animation.h"
#include "engine.h"

// Linear interpolation function
float lerp(float a, float b, float t) { return a + t * (b - a); }

// calculated a
float reverseLerp(float c, float b, float t) { return (c - t * b) / (1 - t); }

// Ease-in quadratic function
float easeInQuad(float t) { return t * t; }

float easeOutQuad(float t) { return t * (2 - t); }

float easeOutCubic(float t) { return (--t) * t * t + 1; }

// Ease-in-out quadratic function
float easeInOutQuad(float t) {
  if (t < 0.5) {
    return 2 * t * t; // Ease-in
  } else {
    return 1 - pow(-2 * t + 2, 2) / 2; // Ease-out
  }
}

static float animationPercentage = 0.0;
int animateToCenter(struct Position positionToCenter,
                    struct Position *updatePosition, float deltaTime,
                    struct Position shiftCenter) {
  static struct Position startPosition;

  struct Position center = shiftPosition(getCenterOfScreen(), shiftCenter);

  // Initialize startPosition on the first call
  if (animationPercentage == 0.0) {
    startPosition = shiftPosition(*updatePosition, center);
  }

  center.x = -1 * center.x;
  center.y = -1 * center.y;

  animationPercentage += deltaTime / ANIMATION_DURATION;
  const float relativePercentage = easeOutQuad(animationPercentage);

  // Calculate current position
  updatePosition->x =
      (int)lerp(startPosition.x, positionToCenter.x, relativePercentage);
  updatePosition->y =
      (int)lerp(startPosition.y, positionToCenter.y, relativePercentage);

  *updatePosition = shiftPosition(*updatePosition, center);

  if (animationPercentage >= 1.0) {
    updatePosition->x = center.x + positionToCenter.x;
    updatePosition->y = center.y + positionToCenter.y;
    animationPercentage = 1.0;
  }

  return animationPercentage;
}

void resetAnimation() { animationPercentage = 0.0; }

float getAnimationPercentage() { return animationPercentage; }

float setAnimationPercentage(float percentage) {
  animationPercentage = percentage;
  return animationPercentage;
}

void goToField(struct Position gridPosition) {
  while (getAnimationPercentage() < 1) {
    animateToCenter(
        getRealTopLeftCorner(gridPosition), gameState.realPosition,
        1.0 / FRAME_RATE,
        (struct Position){-1 * FIELD_WIDTH / 2, -1 * FIELD_HEIGHT / 2});
    renderScreen(&gameState);
    usleep(1000000 / FRAME_RATE);
  }

  resetAnimation();
}

void goToPlayer(int playerIndex) {
  if (gameState.state != PLAYING) {
    return;
  }

  struct Player *player = gameState.players;
  for (int i = 0; i < playerIndex; i++) {
    player = player->nextPlayer;

    if (player->nextPlayer == gameState.players) {
      break;
    }
  }

  goToField(*player->currentField->gridPosition);
}
