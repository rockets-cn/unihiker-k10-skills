#include "unihiker_k10.h"

UNIHIKER_K10 k10;

const int SCREEN_W = 240;
const int SCREEN_H = 320;
const uint32_t COLOR_BG = 0x050712;
const uint32_t COLOR_GRID = 0x18203A;
const uint32_t COLOR_SHIP = 0x34D5FF;
const uint32_t COLOR_SHIP_CORE = 0xFFFFFF;
const uint32_t COLOR_METEOR = 0xFF375F;
const uint32_t COLOR_COIN = 0x4DFF88;
const uint32_t COLOR_TEXT = 0xFFD166;

struct FallingThing {
  int x;
  int y;
  int r;
  int speed;
  bool coin;
};

FallingThing things[7];
int shipX = SCREEN_W / 2;
int shipY = 278;
int score = 0;
int lives = 3;
int level = 1;
bool gameOver = false;
unsigned long lastFrame = 0;

uint32_t wheel(byte pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return ((uint32_t)(255 - pos * 3) << 16) | (pos * 3);
  }
  if (pos < 170) {
    pos -= 85;
    return ((uint32_t)(255 - pos * 3) << 8) | (uint32_t)(pos * 3);
  }
  pos -= 170;
  return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8);
}

void resetThing(int i, bool firstDrop) {
  things[i].x = random(16, SCREEN_W - 16);
  things[i].y = firstDrop ? random(-280, -20) : random(-130, -20);
  things[i].r = random(7, 15);
  things[i].speed = random(3, 7) + level;
  things[i].coin = random(0, 5) == 0;
  if (things[i].coin) {
    things[i].r = 7;
    things[i].speed = max(3, things[i].speed - 1);
  }
}

void resetGame() {
  shipX = SCREEN_W / 2;
  score = 0;
  lives = 3;
  level = 1;
  gameOver = false;
  for (int i = 0; i < 7; i++) {
    resetThing(i, true);
  }
  k10.rgb->brightness(4);
  k10.rgb->write(-1, 0x0033FF);
}

void drawBackground() {
  k10.canvas->canvasRectangle(0, 0, SCREEN_W, SCREEN_H, COLOR_BG, COLOR_BG, true);
  for (int y = 42; y < SCREEN_H; y += 34) {
    k10.canvas->canvasLine(0, y, SCREEN_W, y + 18, COLOR_GRID);
  }
  k10.canvas->canvasText(String("METEOR DASH"), 1, COLOR_TEXT);
  k10.canvas->canvasText(String("Score ") + String(score) + "  Life " + String(lives), 2, 0xFFFFFF);
}

void drawShip() {
  k10.canvas->canvasRectangle(shipX - 12, shipY + 7, 24, 8, 0x1C6CFF, 0x1C6CFF, true);
  k10.canvas->canvasCircle(shipX, shipY, 13, COLOR_SHIP, COLOR_SHIP, true);
  k10.canvas->canvasCircle(shipX, shipY - 2, 5, COLOR_SHIP_CORE, COLOR_SHIP_CORE, true);
  k10.canvas->canvasLine(shipX - 18, shipY + 14, shipX - 7, shipY + 4, COLOR_SHIP);
  k10.canvas->canvasLine(shipX + 18, shipY + 14, shipX + 7, shipY + 4, COLOR_SHIP);
}

void drawThing(FallingThing thing) {
  if (thing.coin) {
    k10.canvas->canvasCircle(thing.x, thing.y, thing.r, COLOR_COIN, COLOR_COIN, true);
    k10.canvas->canvasCircle(thing.x, thing.y, 3, COLOR_BG, COLOR_BG, true);
  } else {
    k10.canvas->canvasCircle(thing.x, thing.y, thing.r, COLOR_METEOR, COLOR_METEOR, true);
    k10.canvas->canvasLine(thing.x - thing.r, thing.y - thing.r, thing.x - thing.r - 12, thing.y - thing.r - 12, 0xFF9F1C);
  }
}

bool hitShip(FallingThing thing) {
  int dx = thing.x - shipX;
  int dy = thing.y - shipY;
  int limit = thing.r + 12;
  return dx * dx + dy * dy < limit * limit;
}

void updateInput() {
  int ax = k10.getAccelerometerX();
  shipX += ax / 35;
  if (k10.buttonA->isPressed()) {
    shipX -= 5;
  }
  if (k10.buttonB->isPressed()) {
    shipX += 5;
  }
  shipX = constrain(shipX, 18, SCREEN_W - 18);
}

void updateThings() {
  level = 1 + score / 12;
  for (int i = 0; i < 7; i++) {
    things[i].y += things[i].speed;
    if (hitShip(things[i])) {
      if (things[i].coin) {
        score += 3;
        k10.rgb->write(-1, 0x00FF55);
      } else {
        lives--;
        k10.rgb->write(-1, 0xFF0000);
        if (lives <= 0) {
          gameOver = true;
          return;
        }
      }
      resetThing(i, false);
    } else if (things[i].y > SCREEN_H + 20) {
      if (!things[i].coin) {
        score++;
      }
      resetThing(i, false);
    }
  }
}

void drawGameOver() {
  k10.canvas->canvasRectangle(0, 0, SCREEN_W, SCREEN_H, 0x12050A, 0x12050A, true);
  k10.canvas->canvasText("CRASH!", 3, COLOR_METEOR);
  k10.canvas->canvasText(String("Final Score ") + String(score), 5, COLOR_TEXT);
  k10.canvas->canvasText("Press A+B", 7, 0xFFFFFF);
  k10.canvas->canvasText("to restart", 8, 0xFFFFFF);
  k10.canvas->updateCanvas();
  k10.rgb->brightness(6);
  k10.rgb->write(-1, wheel((millis() / 8) % 255));
}

void setup() {
  k10.begin();
  k10.initScreen(2);
  k10.creatCanvas();
  randomSeed((unsigned long)analogRead(P1) + millis());
  resetGame();
}

void loop() {
  if (millis() - lastFrame < 55) {
    return;
  }
  lastFrame = millis();

  if (gameOver) {
    drawGameOver();
    if (k10.buttonAB->isPressed()) {
      resetGame();
    }
    return;
  }

  updateInput();
  updateThings();
  drawBackground();
  for (int i = 0; i < 7; i++) {
    drawThing(things[i]);
  }
  drawShip();
  k10.canvas->updateCanvas();
}
