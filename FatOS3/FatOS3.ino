#include <Adafruit_ILI9341.h>
#include "charSet.h"

#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

byte screen[30][40] = {};
byte sizes[30][40] = {};
bool dirty[30][40] = {};

int pColor = ILI9341_BLACK;
int sColor = ILI9341_WHITE;

void setup() {
  tft.begin();
  tft.setRotation(0);
  for (byte y = 0; y < 40; y++) {
    for (byte x = 0; x < 30; x++) {
      screen[x][y] = 0;
      dirty[x][y] = 0;
      sizes[x][y] = 1;
    }
  }
  for (int c = 0; c <= 26; c++) {
    setChar(c + 1, 1, c);
  }

  for (int c = 27; c <= 43; c++) {
    setChar(c - 26, 2, c);
  }

  for (int c = 44; c <= 57; c++) {
    setChar(c - 43, 3, c);
  }
  pColor = ILI9341_RED;
}

void loop() {
  drawScreen();
}

void drawScreen() {
  for (byte dy = 0; dy < 40; dy++) {
    for (byte dx = 0; dx < 30; dx++) {
      if (dirty[dx][dy]) {
        dirty[dx][dy] = 0;
        byte size = sizes[dx][dy];
        drawChar(dx * 8, dy * 8, screen[dx][dy], size);
      }
    }
  }
}

void setChar(int x, int y, byte val) {
  screen[x][y] = val;
  dirty[x][y] = 1;
}

void drawChar(int wx, int wy, byte chr, byte size) {
  for (byte yC = 0; yC < 8; yC++) {
    for (byte xC = 0; xC < 8; xC++) {
      if (bitRead(characters[chr][yC], 7 - xC)) {
        tft.fillRect(wx + xC * size, wy + yC * size, size, size, pColor);
      } else {
        tft.fillRect(wx + xC * size, wy + yC * size, size, size, sColor);
      }
    }
  }
}





