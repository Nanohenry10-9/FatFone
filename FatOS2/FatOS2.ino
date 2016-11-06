//#include <PinChangeInt.h>

#include <Adafruit_ILI9341.h>
#include <Adafruit_FONA.h>
#include <Adafruit_FT6206.h>
#include <SoftwareSerial.h>

Adafruit_FT6206 ts = Adafruit_FT6206();

#define TFT_CS 10
#define TFT_DC 9
#define TFT_BL 5
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define LOCK_PIN A0
#define LED_R A1
#define LED_G A2
#define LED_B A3
#define SD_CS 4

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST -1
#define FONA_RI 6
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

#define lightgrey     ILI9341_LIGHTGREY
#define green         ILI9341_GREEN
#define black         ILI9341_BLACK
#define blue          ILI9341_BLUE
#define navy          ILI9341_NAVY
#define white         ILI9341_WHITE
#define red           ILI9341_RED
#define cyan          ILI9341_CYAN
#define darkgreen     ILI9341_DARKGREEN
#define maroon        ILI9341_MAROON
#define greenyellow   ILI9341_GREENYELLOW
#define darkgrey      ILI9341_DARKGREY
#define yellow        ILI9341_YELLOW

#define MENU          0
#define APP_PHONE     1
#define APP_SMS       2
#define APP_SETTINGS  3
#define APP_PONG      4

#define KEYPAD_CHARS  -2
#define KEYPAD_NUMS   -1

#define timeX 60
#define timeY 12

#define batteryX 220
#define batteryY 5

#define appSize 90
byte appLocX[] = {20, 130, 20, 130};
byte appLocY[] = {70, 70, 180, 180};
uint16_t appColor[] = {red, green, lightgrey, black};
char* appName[] = {"Phone", "SMS", "Set", "Pong"};
byte appNameX[] = {15, 25, 25, 20};

byte bl = 20;
byte audio = FONA_EXTAUDIO;
byte volume = 50;

char RTCtime[23];

long updateTimer = 20000;

byte appOnScreen = 0;

bool appExit = false;

char givenPNumber[10] = {' '};
int pNumCount = 0;

void setup() {
  pinMode(TFT_BL, OUTPUT);
  //pinMode(FONA_KEY, OUTPUT);
  //digitalWrite(FONA_KEY, HIGH);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(navy);
  tft.fillRoundRect(10, 10, 220, 135, 10, cyan);
  tft.setTextSize(12);
  tft.setCursor(50, 30);
  tft.setTextColor(black);
  tft.print('F');
  tft.setTextSize(5);
  tft.print(F("at"));
  tft.setCursor(108, 78);
  tft.print(F("ONE"));
  tft.setTextSize(3);
  tft.setCursor(50, 170);
  tft.setTextColor(white);
  tft.print(F("Starting"));
  tft.setCursor(85, 200);
  for (byte i = 0; i <= bl; i++) {
    backlight(i);
    delay(20);
  }
  fonaSerial->begin(4800);
  tft.print('.');
  if (!fona.begin(*fonaSerial)) {
    tft.setTextColor(red);
    tft.print('x');
    while (true) {}
  }
  tft.print('.');
  fona.setAudio(audio);
  tft.print('.');
  fona.setPWM(2000);
  setAllVolumes(volume);
  //pinMode(FONA_RI, INPUT);
  //pinMode(LOCK_PIN, INPUT_PULLUP);
  ts.begin(40);
  tft.print(F("."));
  fona.setPWM(0);
  Serial.begin(9600);
  for (int i = 320; i > 0; i--) {
    tft.drawFastHLine(0, i, 240, cyan);
  }
  draw(MENU);
}

void loop() {
  if (millis() - updateTimer >= 20000) {
    drawTime(blue);
    drawBattery();
    updateTimer = millis();
  }
  if (ts.touched()) {
    TS_Point touchPoint = ts.getPoint();
    touchHandler();
  }
}

void draw(int a) {
  switch (a) {
    case MENU:
      tft.fillScreen(cyan);
      tft.fillRect(0, 0, 240, 50, blue);
      for (byte i = 0; i < 4; i++) {
        tft.fillRect(appLocX[i], appLocY[i], appSize, appSize, appColor[i]);
        drawText(appName[i], (appLocX[i] + appNameX[i]), (appLocY[i] + 35), 2, white, appColor[i]);
      }
      drawTime(blue);
      drawBattery();
      break;
    case APP_PHONE:
      drawTime(red);
      drawBattery();
      tft.fillRect(20, 70, 90, 50, green);
      drawText("PICK UP", 25, 87, 2, white, green);
      tft.fillRect(130, 70, 90, 50, red);
      drawText("END", 160, 87, 2, white, red);
      tft.fillRect(20, 130, 200, 40, lightgrey);
      tft.drawRect(20, 130, 100, 40, black);
      tft.drawRect(120, 130, 100, 40, black);
      drawText("CONTROL", 30, 150, 2, black, lightgrey);
      drawText("KEYPAD", 140, 150, 2, black, lightgrey);
      break;
    case APP_SMS:
      drawTime(green);
      drawBattery();
      break;
    case APP_SETTINGS:
      drawTime(lightgrey);
      drawBattery();
      break;
    case APP_PONG:
      tft.drawFastHLine(0, 160, 240, white);
      break;
    case KEYPAD_NUMS:
      tft.fillRect(0, 160, 240, 320, white);
      tft.drawFastHLine(0, 159, 240, black);
      tft.drawFastHLine(0, 160, 240, black);

      tft.setTextSize(5);
      tft.setTextColor(black, white);
      tft.setCursor(20, 170);
      tft.print(F("1 2 3 4"));
      tft.setCursor(20, 220);
      tft.print(F("5 6 7 8"));
      tft.setCursor(20, 270);
      tft.print(F("9 0 <<"));
      break;
    case KEYPAD_CHARS:
      tft.fillRect(0, 160, 240, 320, white);
      tft.drawFastHLine(0, 229, 240, black);
      tft.drawFastHLine(0, 230, 240, black);
      tft.setTextColor(darkgreen);
      tft.setCursor(160, 240);
      tft.setTextSize(4);
      tft.print(F("OK"));
      tft.setCursor(160, 280);
      tft.setTextColor(red);
      tft.print(F("<<"));
      tft.setCursor(20, 260);
      tft.setTextColor(black, white);
      tft.print(F("<  >"));
      tft.setTextSize(5);
      tft.setTextColor(black, white);
      tft.setCursor(55, 255);
      tft.print('a');
      break;
  }
}

void setAllVolumes(byte vol) {
  fona.setCallVolume(vol);
  fona.setVolume(vol);
}

void openApp(byte a) {
  int x;
  int y;
  int sizeX;
  int sizeY;
  switch (a) {
    case APP_PHONE:
      for (int i = 0; i < 71; i++) {
        if (i % 6 == 0 || i == 70) {
          y = map(i, 0, 70, 70, 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 12, cyan);
          }
          x = map(i, 0, 70, 20, 0);
          sizeX = map(i, 0, 70, 90, 240);
          sizeY = map(i, 0, 70, 90, 50);
          tft.fillRect(x, y, sizeX, sizeY, red);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      phoneApp();
      break;
    case APP_SMS:
      for (int i = 0; i < 71; i++) {
        if (i % 6 == 0 || i == 70) {
          y = map(i, 0, 70, 70, 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 12, cyan);
          }
          x = map(i, 0, 70, 130, 0);
          sizeX = map(i, 0, 70, 90, 240);
          sizeY = map(i, 0, 70, 90, 50);
          tft.fillRect(x, y, sizeX, sizeY, green);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      smsApp();
      break;
    case APP_SETTINGS:
      for (int i = 0; i < 71; i++) {
        if (i % 6 == 0 || i == 70) {
          y = map(i, 0, 70, 180, 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 20, cyan);
          }
          x = map(i, 0, 70, 20, 0);
          sizeX = map(i, 0, 70, 90, 240);
          sizeY = map(i, 0, 70, 90, 50);
          tft.fillRect(x, y, sizeX, sizeY, lightgrey);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      setApp();
      break;
    case APP_PONG:
      for (int i = 0; i < 71; i++) {
        if (i % 6 == 0 || i == 70) {
          y = map(i, 0, 70, 180, 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 20, cyan);
          }
          x = map(i, 0, 70, 130, 0);
          sizeX = map(i, 0, 70, 90, 240);
          sizeY = map(i, 0, 70, 90, 50);
          tft.fillRect(x, y, sizeX, sizeY, black);
        }
      }
      tft.fillRect(0, 50, 240, 270, black);
      pongApp();
      break;
  }
}

void drawText(char* text, byte locX, byte locY, byte size, uint16_t color, uint16_t bgcolor) {
  tft.setCursor(locX, locY);
  tft.setTextColor(color, bgcolor);
  tft.setTextSize(size);
  tft.print(text);
}

void backlight(int a) {
  int b = map(a, 0, 20, 0, 255);
  analogWrite(TFT_BL, b);
}

void touchHandler() {
  TS_Point tPoint = ts.getPoint();
  int x = map(tPoint.x, 0, 240, 240, 0);
  int y = map(tPoint.y, 0, 320, 320, 0);
  Serial.print(x);
  Serial.print('\t');
  Serial.println(y);
  if (x >= 20 && y >= 70 && x <= 110 && y <= 160) {
    appOnScreen = APP_PHONE;
    openApp(APP_PHONE);
  } else if (x >= 130 && y >= 70 && x <= 220 && y <= 160) {
    appOnScreen = APP_SMS;
    openApp(APP_SMS);
  } else if (x >= 20 && y >= 180 && x <= 110 && y <= 270) {
    appOnScreen = APP_SETTINGS;
    openApp(APP_SETTINGS);
  } else if (x >= 130 && y >= 180 && x <= 220 && y <= 270) {
    appOnScreen = APP_PONG;
    openApp(APP_PONG);
  }
}

void drawBattery() {
  tft.fillRect(batteryX, batteryY, 15, 40, white);
  tft.drawRect(batteryX, batteryY, 15, 40, black);
  if (getBattery() <= 10) {
    tft.fillRect((batteryX + 2), (batteryY + 36), 11, 2, red);
  } else if (getBattery() <= 20) {
    tft.fillRect((batteryX + 2), (batteryY + 30), 11, 8, yellow);
  } else if (getBattery() <= 40) {
    tft.fillRect((batteryX + 2), (batteryY + 22), 11, 16, green);
  } else if (getBattery() <= 60) {
    tft.fillRect((batteryX + 2), (batteryY + 14), 11, 24, green);
  } else if (getBattery() <= 80) {
    tft.fillRect((batteryX + 2), (batteryY + 6), 11, 32, green);
  } else if (getBattery() <= 100) {
    tft.fillRect((batteryX + 2), (batteryY + 2), 11, 36, green);
  }
}

byte getBattery() {
  uint16_t bat;
  fona.getBattPercent(&bat);
  return bat;
}

void drawTime(uint16_t bgcolor) {
  fona.getTime(RTCtime, 23);
  tft.setCursor(timeX, timeY);
  tft.setTextSize(4);
  tft.setTextColor(white, bgcolor);
  for (int i = 10; i < 15; i++) {
    tft.print(RTCtime[i]);
  }
}

void phoneApp() {
  draw(APP_PHONE);
  byte page = 0;
  while (appExit == false) {
    if (millis() - updateTimer >= 20000) {
      drawTime(red);
      drawBattery();
      updateTimer = millis();
    }
    if (ts.touched()) {
      TS_Point tPoint = ts.getPoint();
      int x = map(tPoint.x, 0, 240, 240, 0);
      int y = map(tPoint.y, 0, 320, 320, 0);
      if (y <= 40) {
        appExit = true;
      }
      if (x <= 10 || x >= 230) {
        if (page == 0) {
          page = 1;
          tft.fillRect(0, 50, 240, 270, white);
          tft.drawRect(10, 60, 220, 50, black);
          tft.drawRect(11, 61, 218, 48, black);
          tft.fillRect(20, 120, 200, 30, green);
          drawText("CALL", 85, 125, 3, white, green);
          draw(KEYPAD_NUMS);
          tft.setTextSize(3);
          tft.setTextColor(black, white);
          tft.setCursor(25, 75);
          tft.print(givenPNumber);
        } else {
          page = 0;
          tft.fillRect(0, 50, 240, 270, white);
          tft.fillRect(20, 70, 90, 50, green);
          drawText("PICK UP", 25, 87, 2, white, green);
          tft.fillRect(130, 70, 90, 50, red);
          drawText("END", 160, 87, 2, white, red);
        }
      } else {
        TS_Point tPoint = ts.getPoint();
        x = map(tPoint.x, 0, 240, 240, 0);
        y = map(tPoint.y, 0, 320, 320, 0);
        if (page == 0) {
          if (x >= 20 && y >= 70 && x <= 110 && y <= 120) {
            drawText("PICK UP", 25, 87, 2, black, green);
            while (ts.touched()) {}
            drawText("PICK UP", 25, 87, 2, white, green);
            fona.pickUp();
          } else if (x >= 130 && y >= 70 && x <= 220 && y <= 120) {
            drawText("END", 160, 87, 2, black, red);
            while (ts.touched()) {}
            drawText("END", 160, 87, 2, white, red);
            fona.hangUp();
          }
        } else {
          if (x >= 0 && y >= 160 && x <= 75 && y <= 220) {
            if (pNumCount < 9) {
              givenPNumber[pNumCount] = '1';
              pNumCount++;
            }
          } else if (x >= 75 && y >= 160 && x <= 120 && y <= 220) {
            if (pNumCount < 9) {
              givenPNumber[pNumCount] = '2';
              pNumCount++;
            }
          } else if (x >= 120 && y >= 160 && x <= 185 && y <= 220) {
            if (pNumCount < 9) {
              givenPNumber[pNumCount] = '3';
              pNumCount++;
            }
          } else if (x >= 185 && y >= 160 && x <= 240 && y <= 220) {
            if (pNumCount < 9) {
              givenPNumber[pNumCount] = '4';
              pNumCount++;
            }
          } else if (x >= 0 && y >= 220 && x <= 75 && y <= 260) {
            if (pNumCount < 9) {
              givenPNumber[pNumCount] = '5';
              pNumCount++;
            }
          } else if (x >= 75 && y >= 220 && x <= 120 && y <= 260) {
            if (pNumCount < 9) {
              givenPNumber[pNumCount] = '6';
              pNumCount++;
            }
          } else if (x >= 120 && y >= 220 && x <= 186 && y <= 260) {
            if (pNumCount < 9) {
              givenPNumber[pNumCount] = '7';
              pNumCount++;
            }
          } else if (x >= 185 && y >= 220 && x <= 240 && y <= 260) {
            if (pNumCount < 9) {
              givenPNumber[pNumCount] = '8';
              pNumCount++;
            }
          } else if (x >= 0 && y >= 260 && x <= 75 && y <= 320) {
            if (pNumCount < 9) {
              givenPNumber[pNumCount] = '9';
              pNumCount++;
            }
          } else if (x >= 75 && y >= 260 && x <= 120 && y <= 320) {
            if (pNumCount < 9) {
              givenPNumber[pNumCount] = '0';
              pNumCount++;
            }
          } else if (x >= 120 && y >= 260 && x <= 240 && y <= 320) {
            if (pNumCount > 0) {
              pNumCount--;
              givenPNumber[pNumCount] = ' ';
            }
          } else if (x >= 20 && y >= 120 && x <= 220 && y <= 150) {
            drawText("CALL", 85, 125, 3, black, green);
            while (ts.touched()) {}
            drawText("CALL", 85, 125, 3, white, green);
            fona.callPhone(givenPNumber);
            for (int i = 0; i < 9; i++) {
              givenPNumber[i] = ' ';
            }
            pNumCount = 0;
            tft.setTextSize(3);
            tft.setTextColor(black, white);
            tft.setCursor(25, 75);
            tft.print(givenPNumber);
          }
          if (y >= 160) {
            tft.setTextSize(3);
            tft.setTextColor(black, white);
            tft.setCursor(25, 75);
            tft.print(givenPNumber);
          }
        }
      }
      while (ts.touched()) {}
    }
  }
  appExit = false;
  exitApp();
}

void smsApp() {
  draw(APP_SMS);
  while (appExit == false) {
    if (ts.touched()) {
      TS_Point tPoint = ts.getPoint();
      int x = map(tPoint.x, 0, 240, 240, 0);
      int y = map(tPoint.y, 0, 320, 320, 0);
      if (tPoint.y <= 30) {
        appExit = true;
      }
    }
  }
  appExit = false;
  exitApp();
}

void setApp() {
  draw(APP_SETTINGS);
  while (appExit == false) {
    if (ts.touched()) {
      TS_Point tPoint = ts.getPoint();
      int x = map(tPoint.x, 0, 240, 240, 0);
      int y = map(tPoint.y, 0, 320, 320, 0);
      if (tPoint.y <= 30) {
        appExit = true;
      }
    }
  }
  appExit = false;
  exitApp();
}

void pongApp() {
  draw(APP_PONG);
  while (appExit == false) {
    if (ts.touched()) {
      TS_Point tPoint = ts.getPoint();
      int x = map(tPoint.x, 0, 240, 240, 0);
      int y = map(tPoint.y, 0, 320, 320, 0);
      if (tPoint.y <= 30) {
        appExit = true;
      }
    }
  }
  appExit = false;
  exitApp();
}

void exitApp() {
  for (int i = 320; i > 0; i--) {
    tft.drawFastHLine(0, i, 240, cyan);
  }
  draw(MENU);
}










