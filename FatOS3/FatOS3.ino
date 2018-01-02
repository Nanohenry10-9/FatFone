#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include "charSet.h"

#include <SD.h>

#define TFT_CS 10
#define TFT_DC 9
#define TFT_BL 5
#define SD_CS 4
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#include <Adafruit_FONA.h>
#include <SoftwareSerial.h>
#define FONA_RX 2
#define FONA_TX 12
#define FONA_RST 4
#define FONA_RI 3
#define FONA_KEY A0
#define FONA_PWR A1
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

byte screen[30][40] = {};
byte sizes[30][40] = {};
bool dirty[30][40] = {};
byte colors[30][40] = {};

uint16_t clut[] = {ILI9341_BLACK, ILI9341_WHITE, ILI9341_RED, ILI9341_GREEN, ILI9341_BLUE, ILI9341_YELLOW, ILI9341_NAVY, ILI9341_MAROON};

byte curColor = 0b00000000;
byte curSizes = 0b00010001;

#define LOGO 0
#define MENU 1

char timeC[5] = {"00:00"};
byte battP = 50;

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(0);
  for (byte y = 0; y < 40; y++) {
    for (byte x = 0; x < 30; x++) {
      screen[x][y] = 0;
      dirty[x][y] = 0;
      sizes[x][y] = 0b00010001;
      colors[x][y] = 0b00000100;
    }
  }
  draw(LOGO);
  init_phone();
}

void loop() {
  drawScreen();
}

void draw(byte a) {
  switch (a) {
    case LOGO:
      tft.fillScreen(ILI9341_NAVY);
      curColor = 0b00010110;
      curSizes = 0b01000100;
      setChar(3, 3, 27);
      setChar(7, 3, 31);
      setChar(11, 3, 31);
      setChar(15, 3, 31);
      setChar(19, 3, 31);
      setChar(23, 3, 28);

      setChar(3, 14, 29);
      setChar(7, 14, 33);
      setChar(11, 14, 33);
      setChar(15, 14, 33);
      setChar(19, 14, 33);
      setChar(23, 14, 30);

      setChar(3, 7, 34);
      setChar(3, 11, 34);
      setChar(23, 7, 32);
      setChar(23, 11, 32);

      curColor = 0b00000110;
      curSizes = 0b01101010;
      setChar(7, 5, 6);
      curColor = 0b00010110;
      curSizes = 0b01000100;
      setChar(12, 6, 1);
      setChar(16, 6, 20);
      setChar(20, 6, 0);
      setChar(12, 10, 15);
      setChar(16, 10, 14);
      setChar(20, 10, 5);
      break;
    case MENU:
      /*fillRect(0, 0, 30, 3, 0b00000100);
      fillRect(0, 3, 30, 40, 0b00000001);
      curColor = 0b00010100;
      curSizes = 0b00010001;
      getTime();
      getBatt();
      print(1, 1, timeC, 5);
      printNum(25, 1, battP, 3);
      print(28, 1, "%", 1);
      drawSqr(3, 6, 13, 16, 0b00010100, 0);
      drawSqr(16, 6, 26, 16, 0b00010010, 0);
      drawSqr(3, 19, 13, 29, 0b00010011, 0);
      drawSqr(16, 19, 26, 29, 0b00010101, 0);*/
      readScreenFromSD("/apps/menu/menu.txt");
      break;
  }
  drawScreen();
}

bool readScreenFromSD(char filename[]) {
  Serial.print("Reading file ");
  Serial.print(filename);
  Serial.print("... ");
  File screenData = SD.open(filename, FILE_READ);
  if (!screenData) {
    Serial.print("failure");
    return false;
  }
  byte readX = 0;
  byte readY = 0;
  uint16_t num = 0;
  char cur = 0;
  while (screenData.available()) {
    cur = screenData.read();
    if (isDigit(cur)) {
      num = num * 10 + cur - '0';
    } else if (cur == '\n') {
      readX = 0;
      readY++;
    } else if (cur = ' ') {
      screen[readX][readY] = num >> 8;
      colors[readX][readY] = num & 0b11111111;
      dirty[readX][readY] = 1;
      sizes[readX][readY] = 0b00010001;
      num = 0;
      readX++;
    }
  }
  screenData.close();
  Serial.println("success");
  return true;
}

void fillRect(byte x1, byte y1, byte x2, byte y2, byte color) {
  for (byte y = y1; y < y2; y++) {
    for (byte x = x1; x < x2; x++) {
      screen[x][y] = 0;
      dirty[x][y] = 1;
      sizes[x][y] = 0b00010001;
      colors[x][y] = color;
    }
  }
}

void drawSqr(byte x1, byte y1, byte x2, byte y2, byte colorSet, bool border) {
  curColor = colorSet;
  if (border) {
      setChar(x1, y1, 27);
      setChar(x1, y2, 29);
      setChar(x2, y1, 28);
      setChar(x2, y2, 30);
      for (byte x = x1 + 1; x < x2; x++) {
        setChar(x, y1, 31);
        setChar(x, y2, 33);
      }
      for (byte y = y1 + 1; y < y2; y++) {
        setChar(x1, y, 34);
        setChar(x2, y, 32);
      }
      for (byte y = y1 + 1; y < y2; y++) {
        for (byte x = x1 + 1; x < x2; x++) {
          setChar(x, y, 0);
        }
      }
  } else {
    for (byte y = y1; y <= y2; y++) {
      for (byte x = x1; x <= x2; x++) {
        setChar(x, y, 0);
      }
    }
  }
}

void getTime() {
  char buff[23];
  fona.getTime(buff, 23);
  for (byte i = 10; i < 15; i++) {
    timeC[i - 10] = buff[i];
  }
}

void getBatt() {
  uint16_t batt;
  fona.getBattPercent(&batt);
  battP = batt;
}

void init_phone() {
  curSizes = 0b10011001;
  byte clk = 0;
  clk = clkAdv(clk);
  delay(200);
  fonaSerial->begin(4800);
  clk = clkAdv(clk);
  delay(200);
  if (!fona.begin(*fonaSerial)) {
    curColor = 0b00100110;
    setChar(11, 23, 38);
    drawScreen();
    while (1);
  }
  pinMode(TFT_BL, OUTPUT);
  clk = clkAdv(clk);
  delay(200);
  analogWrite(TFT_BL, 200);
  clk = clkAdv(clk);
  delay(200);
  pinMode(FONA_PWR, INPUT);
  clk = clkAdv(clk);
  delay(200);
  pinMode(FONA_KEY, OUTPUT);
  clk = clkAdv(clk);
  delay(200);
  digitalWrite(FONA_KEY, HIGH);
  clk = clkAdv(clk);
  delay(200);
  if (!SD.begin(SD_CS)) {
    curColor = 0b00100110;
    setChar(11, 23, 38);
    drawScreen();
    Serial.println("SD failure");
    while (1);
  }
  clk = clkAdv(clk);
  delay(200);
  curColor = 0b00110110;
  setChar(11, 23, 39);
  drawScreen();
  delay(1000);
  draw(MENU);
}

byte clkAdv(byte clock) {
  setChar(11, 23, clock + 40);
  drawScreen();
  clock = (clock + 1) % 4;
  return clock;
}

byte digits(int val) {
  byte res = 0;
  while (val >= 1) {
    val /= 10;
  }
  return res;
}

void print(byte x, byte y, char text[], byte len) {
  for (byte i = 0; i < len; i++) {
    if (text[i] >= 65 && text[i] <= 90) {
      setChar(x + i, y, text[i] - 'A' + 1);
    } else if (isDigit(text[i])) {
      setChar(x + i, y, text[i] - 4);
    } else if (text[i] == ' ') {
      setChar(x + i, y, 0);
    } else if (text[i] == '*') {
      setChar(x + i, y, 56);
    } else if (text[i] == '.') {
      setChar(x + i, y, 35);
    } else if (text[i] == ':') {
      setChar(x + i, y, 58);
    } else if (text[i] == '%') {
      setChar(x + i, y, 59);
    } else {
      setChar(x + i, y, text[i]);
    }
  }
}

void printNum(byte x, byte y, int num, byte len) {
  byte numChars[len] = {};
  for (byte i = 0; i < len; i++) {
    numChars[i] = num % 10;
    num /= 10;
  }
  for (byte i = 0; i < len; i++) {
    setChar(x + i, y, numChars[len - i - 1] + 44);
  }
}

byte getBitsFromByte(byte a, bool upper) {
  if (upper) {
    return bitRead(a, 7) * 8 + bitRead(a, 6) * 4 + bitRead(a, 5) * 2 + bitRead(a, 4);
  } else {
    return bitRead(a, 3) * 8 + bitRead(a, 2) * 4 + bitRead(a, 1) * 2 + bitRead(a, 0);
  }
}

void drawScreen() {
  for (byte dy = 0; dy < 40; dy++) {
    for (byte dx = 0; dx < 30; dx++) {
      if (dirty[dx][dy]) {
        dirty[dx][dy] = 0;
        drawChar(dx * 8, dy * 8, screen[dx][dy], getBitsFromByte(sizes[dx][dy], 1), getBitsFromByte(sizes[dx][dy], 0), clut[getBitsFromByte(colors[dx][dy], 1)], clut[getBitsFromByte(colors[dx][dy], 0)]);
      }
    }
  }
}

void setChar(int x, int y, byte val) {
  screen[x][y] = val;
  dirty[x][y] = 1;
  colors[x][y] = curColor;
  sizes[x][y] = curSizes;
}

void drawChar(int wx, int wy, byte chr, byte sX, byte sY, uint16_t pColor, uint16_t sColor) {
  for (byte yC = 0; yC < 8 * sY; yC += sY) {
    for (byte xC = 0; xC < 8 * sX; xC += sX) {
      if (bitRead(characters[chr][yC / sY], 7 - xC / sX)) {
        tft.fillRect(wx + xC, wy + yC, sX, sY, pColor);
      } else {
        tft.fillRect(wx + xC, wy + yC, sX, sY, sColor);
      }
      tft.writecommand(17);
    }
  }
}




