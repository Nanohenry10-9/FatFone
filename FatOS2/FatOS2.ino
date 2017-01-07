#include <Adafruit_ILI9341.h>
#include <Adafruit_FONA.h>
#include <Adafruit_FT6206.h>
#include <SoftwareSerial.h>

Adafruit_FT6206 ts = Adafruit_FT6206();

#define TFT_CS 10
#define TFT_DC 9
#define TFT_BL 5
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

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

#define timeX 74
#define timeY 12

#define batteryX 220
#define batteryY 5

#define SMS_SEND 0
#define SMS_READ 1
#define NUM_FIELD 0
#define MSG_FIELD 1

bool debug = 0;

bool locked = 0;

#define appSize 90
byte appLocX[] = {20, 130/*, 20, 130*/};
byte appLocY[] = {70, 70/*, 180, 180*/};
uint16_t appColor[] = {red, green/*, lightgrey*/};
char* appName[] = {"Phone", "SMS"/*, "Set"*/};
byte appNameX[] = {15, 25/*, 25, 20*/};
byte appAmount = 2;

byte bl = 12;
byte audio = FONA_EXTAUDIO;
byte volume = 50;

char RTCtime[23];

long updateTimer = 20000;

byte appOnScreen = 0;

bool appExit = false;

char givenPNumber[14] = {' '};
int mesLocY[] = {100, 160, 220, 280};

char chars[] = "aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ.!? ";
byte charOnScreenNum = 0;

void setup() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  tft.begin();
  tft.setRotation(0);
  tft.setTextWrap(false);
  tft.fillScreen(navy);
  tft.fillRect(10, 10, 220, 135, /*10,*/ cyan);
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
  fona.begin(*fonaSerial);
  if (!fona.begin(*fonaSerial)) {
    tft.setTextColor(red);
    tft.print('x');
    while (true) {}
  }
  tft.print('.');
  fona.setAudio(audio);
  tft.print('.');
  fona.setPWM(2000);
  fona.setAllVolumes(volume);
  ts.begin(40);
  tft.print(F("."));
  fona.setPWM(0);
  if (fona.getNetworkStatus() == 2) {
    while (fona.getNetworkStatus() == 2) {}
  } else if (fona.getNetworkStatus() == 1) {
    drawText("NETWORK FOUND", 45, 250, 2, white, navy);
  } else {
    drawText("NO NETWORK", 63, 250, 2, white, navy);
  }
  delay(1000);
  for (int i = 320; i > 0; i--) {
    tft.drawFastHLine(0, i, 240, cyan);
  }
  draw(MENU);
}

void loop() {
  if (millis() - updateTimer >= 20000) {
    drawTime(blue);
    //drawBattery();
    updateTimer = millis();
  }
  if (ts.touched()) {
    touchHandler();
  }
}

void draw(int a) {
  switch (a) {
    case MENU:
      tft.fillScreen(cyan);
      tft.fillRect(0, 0, 240, 50, blue);
      for (byte a = 0; a < appAmount; a++) {
        tft.fillRect(appLocX[a], appLocY[a], appSize, appSize, appColor[a]);
        drawText(appName[a], (appLocX[a] + appNameX[a]), (appLocY[a] + 35), 2, white, appColor[a]);
      }
      tft.fillRect(7, 5, 40, 40, red);
      tft.drawRect(7, 5, 40, 40, black);
      drawTime(blue);
      //drawBattery();
      break;
    case APP_PHONE:
      drawTime(red);
      //drawBattery();
      drawText("BACK", 5, 18, 2, white, red);
      tft.fillRect(20, 70, 90, 50, green);
      drawText("PICK UP", 25, 87, 2, white, green);
      tft.fillRect(130, 70, 90, 50, red);
      drawText("END", 157, 87, 2, white, red);
      tft.fillRect(20, 130, 200, 40, lightgrey);
      drawText("KEYPAD", 85, 142, 2, black, lightgrey);
      drawText("VOLUME", 85, 190, 2, black, white);
      tft.drawFastHLine(20, 230, 200, black);
      tft.drawFastVLine(20, 210, 40, black);
      tft.drawFastVLine(220, 210, 40, black);
      tft.drawFastVLine(120, 215, 30, black);
      tft.drawFastVLine(70, 220, 20, black);
      tft.drawFastVLine(170, 220, 20, black);
      tft.fillRect(map(volume, 0, 100, 20, 210), 210, 10, 40, darkgrey);
      break;
    case APP_SMS:
      drawTime(green);
      //drawBattery();
      drawText("BACK", 5, 18, 2, white, green);
      tft.fillRect(0, 50, 60, 40, darkgrey);
      tft.fillRect(180, 50, 60, 40, darkgrey);
      drawText("SEND", 5, 60, 2, white, darkgrey);
      drawText("READ", 188, 60, 2, white, darkgrey);
      tft.drawRect(10, 95, 220, 40, black);
      tft.drawRect(10, 140, 220, 40, black);
      tft.fillRect(60, 50, 120, 40, darkgreen);
      drawText("SEND", 100, 60, 2, white, darkgreen);
      draw(KEYPAD_NUMS);
      break;
    /*case APP_SETTINGS:
      drawText("BACK", 5, 18, 2, white, lightgrey);
      drawTime(lightgrey);
      drawBattery();
      break;
      case APP_PONG:
      tft.drawFastHLine(0, 160, 240, white);
      break;*/
    case KEYPAD_NUMS:
      tft.fillRect(0, 189, 240, 160, white);
      tft.drawFastHLine(0, 189, 240, black);
      tft.drawFastHLine(0, 190, 240, black);
      tft.setTextSize(5);
      tft.setTextColor(black, white);
      tft.setCursor(20, 195);
      tft.print(F("1 2 3 4"));
      tft.setCursor(20, 237);
      tft.print(F("5 6 7 8"));
      tft.setCursor(20, 279);
      tft.print(F("9 0"));
      tft.setCursor(120, 279);
      tft.print(F("<<"));
      tft.setCursor(200, 279);
      tft.print(F("+"));
      break;
    case KEYPAD_CHARS:
      tft.fillRect(0, 189, 240, 160, white);
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
      tft.print(chars[charOnScreenNum]);
      break;
  }
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
      /*case APP_SETTINGS:
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
          break;*/
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
  if (x >= 7 && y >= 5 && x <= 47 && y <= 45) {
    tft.fillRect(8, 6, 38, 38, maroon);
    while (ts.touched()) {}
    tft.fillRect(8, 6, 38, 38, red);
    lock();
  } else if (x >= 20 && y >= 70 && x <= 110 && y <= 160) {
    drawText(appName[0], (appLocX[0] + appNameX[0]), (appLocY[0] + 35), 2, black, appColor[0]);
    while (ts.touched()) {}
    drawText(appName[0], (appLocX[0] + appNameX[0]), (appLocY[0] + 35), 2, white, appColor[0]);
    appOnScreen = APP_PHONE;
    openApp(APP_PHONE);
  } else if (x >= 130 && y >= 70 && x <= 220 && y <= 160) {
    drawText(appName[1], (appLocX[1] + appNameX[1]), (appLocY[1] + 35), 2, black, appColor[1]);
    while (ts.touched()) {}
    drawText(appName[1], (appLocX[1] + appNameX[1]), (appLocY[1] + 35), 2, white, appColor[1]);
    appOnScreen = APP_SMS;
    openApp(APP_SMS);
  }/* else if (x >= 20 && y >= 180 && x <= 110 && y <= 270) {
    drawText(appName[2], (appLocX[2] + appNameX[2]), (appLocY[2] + 35), 2, black, appColor[2]);
    while (ts.touched()) {}
    drawText(appName[2], (appLocX[2] + appNameX[2]), (appLocY[2] + 35), 2, white, appColor[2]);
    appOnScreen = APP_SETTINGS;
    openApp(APP_SETTINGS);
  } else if (x >= 130 && y >= 180 && x <= 220 && y <= 270) {
    appOnScreen = APP_PONG;
    openApp(APP_PONG);
  }*/
}

/*void drawBattery() {
  tft.drawRect(batteryX, batteryY, 15, 40, black);
  tft.drawRect((batteryX + 1), (batteryY + 1), 13, 38, black);
  tft.fillRect((batteryX + 2), (batteryY + 2), 11, 36, white);
  tft.drawFastHLine((batteryX + 5), (batteryY - 2), 5, black);
  tft.drawFastHLine((batteryX + 5), (batteryY - 1), 5, black);
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
  }*/

void drawTime(uint16_t bgcolor) {
  fona.getTime(RTCtime, 23);
  if (bgcolor != blue && bgcolor != cyan) {
    tft.setCursor((timeX + 5), timeY);
  } else {
    tft.setCursor(timeX, timeY);
  }
  tft.setTextSize(4);
  tft.setTextColor(white, bgcolor);
  for (int i = 10; i < 15; i++) {
    tft.print(RTCtime[i]);
  }
}

void phoneApp() {
  draw(APP_PHONE);
  byte page = 0;
  int pNumCount = 0;
  while (appExit == false) {
    if (millis() - updateTimer >= 20000) {
      drawTime(red);
      //drawBattery();
      updateTimer = millis();
    }
    if (ts.touched()) {
      TS_Point tPoint = ts.getPoint();
      int x = map(tPoint.x, 0, 240, 240, 0);
      int y = map(tPoint.y, 0, 320, 320, 0);
      if (y <= 50) {
        drawText("BACK", 5, 18, 2, black, red);
        appExit = true;
        while (ts.touched()) {}
        drawText("BACK", 5, 18, 2, white, red);
      }
      if (page == 0) {
        if (x >= 20 && y >= 70 && x <= 110 && y <= 120) {
          drawText("PICK UP", 25, 87, 2, black, green);
          while (ts.touched()) {}
          fona.pickUp();
          drawText("PICK UP", 25, 87, 2, white, green);
        } else if (x >= 130 && y >= 70 && x <= 220 && y <= 120) {
          drawText("END", 157, 87, 2, black, red);
          while (ts.touched()) {}
          fona.hangUp();
          drawText("END", 157, 87, 2, white, red);
        } else if (x >= 20 && y >= 130 && x <= 220 && y <= 170) {
          drawText("KEYPAD", 85, 142, 2, white, lightgrey);
          while (ts.touched()) {}
          drawText("KEYPAD", 85, 142, 2, black, lightgrey);
          page = 1;
          tft.fillRect(0, 50, 240, 270, white);
          tft.drawRect(10, 60, 220, 50, black);
          tft.drawRect(11, 61, 218, 48, black);
          tft.fillRect(125, 120, 95, 30, green);
          drawText("CALL", 140, 125, 3, white, green);
          tft.fillRect(20, 120, 95, 30, red);
          drawText("BACK", 35, 125, 3, white, red);
          draw(KEYPAD_NUMS);
          tft.setTextSize(3);
          tft.setTextColor(black, white);
          tft.setCursor(25, 75);
          tft.print(givenPNumber);
        } else if (x >= 20 && y >= 200 && x <= 220 && y <= 240) {
          while (ts.touched()) {
            tPoint = ts.getPoint();
            x = map(tPoint.x, 0, 240, 240, 0);
            y = map(tPoint.y, 0, 320, 320, 0);
            if (x >= 20 && x <= 220) {
              tft.fillRect(map(volume, 0, 100, 20, 210), 210, 10, 40, white);
              tft.drawFastHLine(20, 230, 200, black);
              tft.drawFastVLine(20, 210, 40, black);
              tft.drawFastVLine(220, 210, 40, black);
              tft.drawFastVLine(120, 215, 30, black);
              tft.drawFastVLine(70, 220, 20, black);
              tft.drawFastVLine(170, 220, 20, black);
              volume = map(x, 20, 220, 0, 100);
              tft.fillRect(map(volume, 0, 100, 20, 210), 210, 10, 40, darkgrey);
            }
          }
          fona.setAllVolumes(volume);
        }
      } else {
        if (getKPPress() == 1) {
          if (pNumCount < 13) {
            givenPNumber[pNumCount] = '1';
            pNumCount++;
          }
        } else if (getKPPress() == 2) {
          if (pNumCount < 13) {
            givenPNumber[pNumCount] = '2';
            pNumCount++;
          }
        } else if (getKPPress() == 3) {
          if (pNumCount < 13) {
            givenPNumber[pNumCount] = '3';
            pNumCount++;
          }
        } else if (getKPPress() == 4) {
          if (pNumCount < 13) {
            givenPNumber[pNumCount] = '4';
            pNumCount++;
          }
        } else if (getKPPress() == 5) {
          if (pNumCount < 13) {
            givenPNumber[pNumCount] = '5';
            pNumCount++;
          }
        } else if (getKPPress() == 6) {
          if (pNumCount < 13) {
            givenPNumber[pNumCount] = '6';
            pNumCount++;
          }
        } else if (getKPPress() == 7) {
          if (pNumCount < 13) {
            givenPNumber[pNumCount] = '7';
            pNumCount++;
          }
        } else if (getKPPress() == 8) {
          if (pNumCount < 13) {
            givenPNumber[pNumCount] = '8';
            pNumCount++;
          }
        } else if (getKPPress() == 9) {
          if (pNumCount < 13) {
            givenPNumber[pNumCount] = '9';
            pNumCount++;
          }
        } else if (getKPPress() == 0) {
          if (pNumCount < 13) {
            givenPNumber[pNumCount] = '0';
            pNumCount++;
          }
        } else if (getKPPress() == -1) {
          if (pNumCount > 0) {
            pNumCount--;
            givenPNumber[pNumCount] = ' ';
          }
        } else if (getKPPress() == -2) {
          if (pNumCount == 0) {
            givenPNumber[pNumCount] = '+';
            pNumCount++;
          }
        } else if (x >= 120 && y >= 120 && x <= 220 && y <= 150) {
          drawText("CALL", 140, 125, 3, black, green);
          while (ts.touched()) {}
          fona.callPhone(givenPNumber);
          for (int i = 0; i < 9; i++) {
            givenPNumber[i] = ' ';
          }
          pNumCount = 0;
          tft.setTextSize(3);
          tft.setTextColor(black, white);
          tft.setCursor(15, 75);
          tft.print(givenPNumber);
          drawText("CALL", 140, 125, 3, white, green);
        } else if (x >= 20 && y >= 120 && x <= 115 && y <= 150) {
          drawText("BACK", 35, 125, 3, black, red);
          while (ts.touched()) {}
          drawText("BACK", 35, 125, 3, white, red);
          page = 0;
          tft.fillRect(0, 50, 240, 270, white);
          tft.fillRect(20, 70, 90, 50, green);
          drawText("PICK UP", 25, 87, 2, white, green);
          tft.fillRect(130, 70, 90, 50, red);
          drawText("END", 157, 87, 2, white, red);
          tft.fillRect(20, 130, 200, 40, lightgrey);
          drawText("KEYPAD", 85, 142, 2, black, lightgrey);
          drawText("VOLUME", 85, 190, 2, black, white);
          tft.drawFastHLine(20, 230, 200, black);
          tft.drawFastVLine(20, 210, 40, black);
          tft.drawFastVLine(220, 210, 40, black);
          tft.drawFastVLine(120, 215, 30, black);
          tft.drawFastVLine(70, 220, 20, black);
          tft.drawFastVLine(170, 220, 20, black);
          tft.fillRect(map(volume, 0, 100, 20, 210), 210, 10, 40, darkgrey);
        }
        if (y >= 190) {
          tft.setTextSize(3);
          tft.setTextColor(black, white);
          tft.setCursor(25, 75);
          tft.print(givenPNumber);
        }
      }
    }
    while (ts.touched()) {}
  }
  appExit = false;
  for (int i = 0; i < 9; i++) {
    givenPNumber[i] = ' ';
  }
  pNumCount = 0;
  exitApp();
}

void smsApp() {
  draw(APP_SMS);
  byte page = SMS_SEND;
  int pNumCount = 0;
  byte selectedField = NUM_FIELD;
  char charOnScreen = chars[charOnScreenNum];
  char message[21] = {' '};
  int messageIndex = 0;
  while (appExit == false) {
    if (ts.touched()) {
      TS_Point tPoint = ts.getPoint();
      int x = map(tPoint.x, 0, 240, 240, 0);
      int y = map(tPoint.y, 0, 320, 320, 0);
      if (y <= 50) {
        drawText("BACK", 5, 18, 2, black, green);
        appExit = true;
        while (ts.touched()) {}
        drawText("BACK", 5, 18, 2, white, green);
      }
      if (x >= 0 && y >= 50 && x <= 60 && y <= 90) {
        drawText("SEND", 5, 60, 2, black, darkgrey);
        while (ts.touched()) {}
        drawText("SEND", 5, 60, 2, white, darkgrey);
        page = SMS_SEND;
        tft.fillRect(0, 90, 240, 270, white);
        tft.drawRect(10, 95, 220, 40, black);
        tft.drawRect(10, 140, 220, 40, black);
        tft.fillRect(60, 50, 120, 40, darkgreen);
        drawText("SEND", 100, 60, 2, white, darkgreen);
        draw(KEYPAD_NUMS);
      } else if (x >= 180 && y >= 50 && x <= 240 && y <= 90) {
        drawText("READ", 188, 60, 2, black, darkgrey);
        while (ts.touched()) {}
        drawText("READ", 188, 60, 2, white, darkgrey);
        page = SMS_READ;
        tft.fillRect(0, 90, 240, 270, white);
        tft.fillRect(60, 50, 120, 40, darkgreen);
        drawText("REFRESH", 78, 60, 2, white, darkgreen);
        tft.drawFastHLine(20, 150, 200, darkgrey);
        tft.drawFastHLine(20, 210, 200, darkgrey);
        tft.drawFastHLine(20, 270, 200, darkgrey);
        int smsAmount = fona.getNumSMS();
        if (smsAmount > 3) {
          smsAmount = 3;
        }
        if (smsAmount > 0) {
          drawSMS();
        } else {
          tft.setCursor(50, mesLocY[2]);
          tft.print(F("NO MESSAGES"));
        }
      }
      if (page == SMS_SEND) {
        if (x >= 10 && y >= 95 && x <= 230 && y <= 135) {
          selectedField = NUM_FIELD;
          draw(KEYPAD_NUMS);
          tft.setTextSize(3);
          tft.setTextColor(black, white);
          tft.setCursor(25, 105);
          tft.print(givenPNumber);
        } else if (x >= 10 && y >= 140 && x <= 230 && y <= 180) {
          selectedField = MSG_FIELD;
          draw(KEYPAD_CHARS);
          tft.setTextSize(2);
          tft.setTextColor(black, white);
          tft.setCursor(25, 145);
          tft.print(message);
        } else if (x >= 60 && y >= 50 && x <= 180 && y <= 90) {
          drawText("SEND", 100, 60, 2, black, darkgreen);
          while (ts.touched()) {}
          fona.sendSMS(givenPNumber, message);
          for (int i = 0; i < 9; i++) {
            givenPNumber[i] = ' ';
          }
          pNumCount = 0;
          tft.setTextSize(3);
          tft.setTextColor(black, white);
          tft.setCursor(25, 105);
          tft.print(givenPNumber);
          for (int i = 0; i < 20; i++) {
            message[i] = ' ';
          }
          tft.setTextSize(2);
          tft.setTextColor(black, white);
          tft.setCursor(25, 145);
          tft.print(message);
          tft.drawRect(10, 95, 220, 40, black);
          tft.drawRect(10, 140, 220, 40, black);
          drawText("SEND", 100, 60, 2, white, darkgreen);
        }
        if (selectedField == NUM_FIELD) {
          if (getKPPress() == 1) {
            if (pNumCount < 13) {
              givenPNumber[pNumCount] = '1';
              pNumCount++;
            }
          } else if (getKPPress() == 2) {
            if (pNumCount < 13) {
              givenPNumber[pNumCount] = '2';
              pNumCount++;
            }
          } else if (getKPPress() == 3) {
            if (pNumCount < 13) {
              givenPNumber[pNumCount] = '3';
              pNumCount++;
            }
          } else if (getKPPress() == 4) {
            if (pNumCount < 13) {
              givenPNumber[pNumCount] = '4';
              pNumCount++;
            }
          } else if (getKPPress() == 5) {
            if (pNumCount < 13) {
              givenPNumber[pNumCount] = '5';
              pNumCount++;
            }
          } else if (getKPPress() == 6) {
            if (pNumCount < 13) {
              givenPNumber[pNumCount] = '6';
              pNumCount++;
            }
          } else if (getKPPress() == 7) {
            if (pNumCount < 13) {
              givenPNumber[pNumCount] = '7';
              pNumCount++;
            }
          } else if (getKPPress() == 8) {
            if (pNumCount < 13) {
              givenPNumber[pNumCount] = '8';
              pNumCount++;
            }
          } else if (getKPPress() == 9) {
            if (pNumCount < 13) {
              givenPNumber[pNumCount] = '9';
              pNumCount++;
            }
          } else if (getKPPress() == 0) {
            if (pNumCount < 13) {
              givenPNumber[pNumCount] = '0';
              pNumCount++;
            }
          } else if (getKPPress() == -1) {
            if (pNumCount > 0) {
              pNumCount--;
              givenPNumber[pNumCount] = ' ';
            }
          } else if (getKPPress() == -2) {
            if (pNumCount == 0) {
              givenPNumber[pNumCount] = '+';
              pNumCount++;
            }
          }
          if (y >= 190) {
            tft.setTextSize(3);
            tft.setTextColor(black, white);
            tft.setCursor(15, 105);
            tft.print(givenPNumber);
          }
        } else {
          if (x >= 20 && y >= 230 && x <= 80 && y <= 320) {
            if (charOnScreenNum == 0) {
              charOnScreenNum = 55;
            } else {
              charOnScreenNum = (charOnScreenNum - 1);
            }
            charOnScreen = chars[charOnScreenNum];
            tft.setTextSize(5);
            tft.setTextColor(black, white);
            tft.setCursor(55, 255);
            tft.print(charOnScreen);
          } else if (x >= 80 && y >= 230 && x <= 160 && y <= 320) {
            charOnScreenNum = (charOnScreenNum + 1);
            if (charOnScreenNum >= 56) {
              charOnScreenNum = 0;
            }
            charOnScreen = chars[charOnScreenNum];
            tft.setTextSize(5);
            tft.setTextColor(black, white);
            tft.setCursor(55, 255);
            tft.print(charOnScreen);
          } else if (x >= 160 && y >= 240 && x <= 200 && y <= 270) {
            message[messageIndex] = charOnScreen;
            if (messageIndex <= 15) {
              messageIndex = (messageIndex + 1);
              tft.setTextSize(2);
              tft.setTextColor(black, white);
              tft.setCursor(25, 145);
              tft.print(message);
            }
          } else if (x >= 160 && y >= 280 && x <= 320 && y <= 320 && messageIndex >= 1) {
            messageIndex = (messageIndex - 1);
            message[messageIndex] = ' ';
            tft.setTextSize(2);
            tft.setTextColor(black, white);
            tft.setCursor(25, 145);
            tft.print(message);
          }
        }
      } else if (page == SMS_READ) {
        if (x >= 60 && y >= 50 && x <= 180 && y <= 90) {
          drawText("REFRESH", 78, 60, 2, black, darkgreen);
          while (ts.touched()) {}
          drawText("REFRESH", 78, 60, 2, white, darkgreen);
          tft.fillRect(0, 90, 240, 270, white);
          tft.drawFastHLine(20, 150, 200, darkgrey);
          tft.drawFastHLine(20, 210, 200, darkgrey);
          tft.drawFastHLine(20, 270, 200, darkgrey);
          int smsAmount = fona.getNumSMS();
          if (smsAmount > 0) {
            drawSMS();
          } else {
            tft.setCursor(50, mesLocY[2]);
            tft.print(F("NO MESSAGES"));
          }
        }
      }
      while (ts.touched()) {}
    }
  }
  appExit = false;
  for (int i = 0; i < 9; i++) {
    givenPNumber[i] = ' ';
  }
  for (int i = 0; i < 15; i++) {
    message[i] = ' ';
  }
  pNumCount = 0;
  exitApp();
}

void drawSMS() {
  char tempBuff1[21] = {' '};
  char tempBuff2[21] = {' '};
  uint16_t lengthBuff = 0;

  char message1[21] = {' '};
  char message2[21] = {' '};
  char message3[21] = {' '};

  /*char time1[21] = {' '};
    char time2[21] = {' '};
    char time3[21] = {' '};*/

  char sender1[21] = {' '};
  char sender2[21] = {' '};
  char sender3[21] = {' '};

  uint16_t smsLen;

  tft.setTextSize(2);
  tft.setTextColor(black, white);
  tft.setCursor(50, mesLocY[0]);
  tft.print(F("Loading..."));
  tft.setCursor(50, mesLocY[1]);
  tft.print(F("Loading..."));
  tft.setCursor(50, mesLocY[2]);
  tft.print(F("Loading..."));
  for (uint16_t i = 0; i < 50; i++) {
    fona.readSMS(i, tempBuff1, 21, &lengthBuff);
    if (lengthBuff > 0) {
      fona.getSMSSender(i, tempBuff2, 21);
      memcpy(message3, message2, 21 * sizeof(char));
      memcpy(message2, message1, 21 * sizeof(char));
      memcpy(message1, tempBuff1, 21 * sizeof(char));
      memcpy(sender3, sender2, 21 * sizeof(char));
      memcpy(sender2, sender1, 21 * sizeof(char));
      memcpy(sender1, tempBuff2, 21 * sizeof(char));
    }
  }
  tft.setCursor(10, mesLocY[0]);
  tft.print(message1);
  tft.print(F("                "));
  tft.setCursor(10, (mesLocY[0] + 25));
  tft.print(sender1);

  tft.setCursor(10, mesLocY[1]);
  tft.print(message2);
  tft.print(F("                "));
  tft.setCursor(10, (mesLocY[1] + 25));
  tft.print(sender2);

  tft.setCursor(10, mesLocY[2]);
  tft.print(message3);
  tft.print(F("                "));
  tft.setCursor(10, (mesLocY[2] + 25));
  tft.print(sender3);
}

/*void setApp() {
  draw(APP_SETTINGS);
  while (appExit == false) {
    if (ts.touched()) {
      TS_Point tPoint = ts.getPoint();
      int x = map(tPoint.x, 0, 240, 240, 0);
      int y = map(tPoint.y, 0, 320, 320, 0);
      if (y <= 50) {
        drawText("BACK", 5, 18, 2, black, lightgrey);
        appExit = true;
        while (ts.touched()) {}
        drawText("BACK", 5, 18, 2, white, lightgrey);
      }
      while (ts.touched()) {}
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
      if (y <= 50) {
        appExit = true;
      }
    }
  }
  appExit = false;
  exitApp();
  }*/

void exitApp() {
  for (int i = 320; i > 0; i--) {
    tft.drawFastHLine(0, i, 240, cyan);
  }
  draw(MENU);
}

void lock() {
  for (int i = bl; i >= 0; i--) {
    backlight(i);
    delay(40);
  }
  /*tft.setCursor(50, 50);
    tft.setTextSize(5);
    tft.setTextColor(black, white);*/
  bool unlocked = false;
  while (!unlocked) {
    if (ts.touched()) {
      if (getTouchPart() == 1) {
        while (getTouchPart() == 1 && ts.touched()) {}
        if (ts.touched() && getTouchPart() == 2) {
          while (getTouchPart() == 2 && ts.touched()) {}
          if (ts.touched() && getTouchPart() == 4) {
            while (getTouchPart() == 4 && ts.touched()) {}
            if (ts.touched() && getTouchPart() == 3) {
              while (getTouchPart() == 3 && ts.touched()) {}
              if (ts.touched() && getTouchPart() == 1) {
                while (getTouchPart() == 1 && ts.touched()) {}
                if (ts.touched() && getTouchPart() == 2) {
                  unlocked = true;
                }
              }
            }
          }
        }
      }
    }
    /*tft.print(getTouchPart());
      tft.setCursor(50, 50);*/
  }
  for (int i = 0; i <= bl; i++) {
    backlight(i);
    delay(10);
  }
  while (ts.touched()) {}
}

int getTouchPart() {
  TS_Point tPoint = ts.getPoint();
  int x = map(tPoint.x, 0, 240, 240, 0);
  int y = map(tPoint.y, 0, 320, 320, 0);
  if (!ts.touched()) {
    return 0;
  } else if (x <= 120 && y <= 160) {
    return 1;
  } else if (x >= 120 && y <= 160) {
    return 2;
  } else if (x <= 120 && y >= 160) {
    return 3;
  } else if (x >= 120 && y >= 160) {
    return 4;
  }
}

int getKPPress() {
  if (ts.touched()) {
    TS_Point tPoint = ts.getPoint();
    int x = map(tPoint.x, 0, 240, 240, 0);
    int y = map(tPoint.y, 0, 320, 320, 0);
    if (y >= 190) {
      if (x >= 0 && y >= 185 && x <= 75 && y <= 245) {
        return 1;
      } else if (x >= 75 && y >= 185 && x <= 120 && y <= 245) {
        return 2;
      } else if (x >= 120 && y >= 185 && x <= 185 && y <= 245) {
        return 3;
      } else if (x >= 185 && y >= 185 && x <= 240 && y <= 245) {
        return 4;
      } else if (x >= 0 && y >= 237 && x <= 75 && y <= 277) {
        return 5;
      } else if (x >= 75 && y >= 237 && x <= 120 && y <= 277) {
        return 6;
      } else if (x >= 120 && y >= 237 && x <= 186 && y <= 277) {
        return 7;
      } else if (x >= 185 && y >= 237 && x <= 240 && y <= 277) {
        return 8;
      } else if (x >= 0 && y >= 269 && x <= 75 && y <= 329) {
        return 9;
      } else if (x >= 75 && y >= 269 && x <= 120 && y <= 329) {
        return 0;
      } else if (x >= 120 && y >= 269 && x <= 186 && y <= 329) {
        return -1;
      } else if (x >= 185 && y >= 269 && x <= 240 && y <= 329) {
        return -2;
      }
    }
  }
}










