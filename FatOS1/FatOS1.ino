#include <EEPROM.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FONA.h>
#include <Adafruit_FT6206.h>
#include <SoftwareSerial.h>

Adafruit_FT6206 ts = Adafruit_FT6206();

#define TFT_CS 10
#define TFT_DC 9
#define TFT_BL 5
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define LOCK_PIN 7

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST -1
#define FONA_RI 6
#define FONA_KEY 8
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

#define KEYPAD_CHARS -3
#define LOCKTIME -2
#define KEYPAD_NUMBERS -1
#define LOCKSCREEN 0
#define MENU 1
#define SETTINGS 2
#define PHONE 3
#define MESSAGES 4
#define CONTACTS 5
#define PONG 6

#define LSTextX 60
#define LSTextY 20

#define clockX 60
#define clockY 12

#define batteryX 220
#define batteryY 5

#define NOT_CALLING 0
#define CALLING_TO 1
#define CALL_FROM 2

#define MESSAGE 0
#define NUMBER 1

byte password[] = {0, 0, 0, 0};
byte givenPassword[] = {' ', ' ', ' ', ' '};

byte bl = 12;
byte volume = 50;
bool screenDimmed = false;
byte dimmedBL;
byte audio = FONA_EXTAUDIO;

bool phoneLocked = false;

int screen = 0;
int lastScreen = 1;

char RTCtime[23];
long int updateTimer = millis();

const int appX[] = {20, 20, 20, 20/*, 20*/};
const int appY[] = {70, 120, 170, 220/*, 270*/};
const char* appName[5] = {"Settings", "Phone", "Messages"/*, "Pong"*/};
const unsigned int appColor[] = {darkgrey, red, darkgreen/*, black*/};

long idleTimer;
const int idleTimeout = 60000;

long uptime = millis();

bool hasStarted = false;

uint16_t bat;
int batry;
int oldBatry;
bool charging = false;

char givenPNumber[10] = {' '};

int netStat;
int oldNetStat;

byte callStat = NOT_CALLING;
byte oldCallStat = NOT_CALLING;
char incomingCallNumber[34] = {' '};

void setup() {
  pinMode(TFT_BL, OUTPUT);
  pinMode(FONA_KEY, OUTPUT);
  digitalWrite(FONA_KEY, HIGH);
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
  for (int i = 0; i <= bl; i++) {
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
  setAllVolumes(volume);
  pinMode(FONA_RI, INPUT);
  pinMode(LOCK_PIN, INPUT_PULLUP);
  ts.begin(40);
  tft.print(F("."));
  fona.playToolkitTone(6, 500);
  delay(1000);
  draw(LOCKSCREEN, 2);
  idleTimer = millis();
  oldBatry = getBattery();
}

void loop() {
  if (phoneLocked) {
    lockscreenOpen();
    givenPassword[0] = ' ';
    givenPassword[1] = ' ';
    givenPassword[2] = ' ';
    givenPassword[3] = ' ';
    if (hasStarted) {
      draw(lastScreen, 1);
    } else {
      draw(MENU, 1);
      hasStarted = true;
    }
  }
  drawTime(1, blue, false);
  if (ts.touched()) {
    touchHandler(MENU);
  }
  if (millis() - idleTimer >= (idleTimeout - 7000)) {
    screenDimmed = true;
    dimmedBL = (bl / 2);
    backlight(dimmedBL);
    if (millis() - idleTimer >= idleTimeout) {
      lock();
      idleTimer = millis();
      screenDimmed = false;
    }
  }
  if (batteryUpdated()) {
    drawBattery();
  }
  if (digitalRead(LOCK_PIN) == 0) {
    lock();
  }
  if (checkNetStat() == true) {
    printNetStat();
  }
}

void draw(int a, int b) {
  if (a != KEYPAD_NUMBERS && a != KEYPAD_CHARS && b != 0) {
    for (int i = bl; i >= 0; i--) {
      backlight(i);
      delay(10);
    }
    screen = a;
  }
  if (a != KEYPAD_NUMBERS && a != KEYPAD_CHARS) {
    updateTimer = 60000;
  }
  switch (a) {
    case LOCKTIME:
      tft.setTextColor(black, cyan);
      tft.setTextSize(6);
      tft.fillRoundRect(18, 10, 200, 65, 8, cyan);
      tft.setCursor(35, 20);
      for (int i = 10; i < 15; i++) {
        tft.print(RTCtime[i]);
      }
      tft.setTextColor(white, blue);
      tft.setTextSize(3);
      tft.setCursor(40, 90);
      tft.print(F("BATT:"));
      if (getBattery() >= 95) {
        tft.print(F("Full"));
      } else {
        tft.print(F(" "));
        tft.print(getBattery());
        tft.print(F("%"));
      }
      break;
    case KEYPAD_NUMBERS:
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
    case LOCKSCREEN:
      if (b == 2) {
        tft.fillScreen(blue);
      }
      tft.fillRect(5, 5, 230, 70, white);
      tft.drawRect(5, 5, 230, 70, black);
      draw(KEYPAD_NUMBERS, 0);
      drawTime(0, blue, true);
      tft.setTextSize(5);
      tft.setTextColor(black, white);
      tft.setCursor(LSTextX, LSTextY);
      if (givenPassword[0] != ' ') {
        tft.print(givenPassword[0]);
      }
      if (givenPassword[1] != ' ') {
        tft.print(givenPassword[1]);
      }
      if (givenPassword[2] != ' ') {
        tft.print(givenPassword[2]);
      }
      if (givenPassword[3] != ' ') {
        tft.print(givenPassword[3]);
      }
      phoneLocked = true;
      break;
    case MENU:
      tft.fillScreen(cyan);
      tft.fillRect(0, 0, 240, 50, blue);
      for (int i = 0; i < 4; i++) {
        tft.setCursor(appX[i], appY[i]);
        tft.setTextColor(appColor[i], cyan);
        tft.setTextSize(3);
        tft.print(appName[i]);
      }
      lastScreen = MENU;
      drawBattery();
      printNetStat();
      drawTime(1, blue, true);
      break;
    case SETTINGS:
      tft.setCursor(7, 80);
      tft.setTextColor(black, white);
      tft.setTextSize(2);
      tft.print(F("Backlight"));
      tft.fillRect(130, 70, 100, 50, darkgrey);
      tft.drawRect(130, 70, 50, 50, black);
      tft.drawRect(180, 70, 50, 50, black);
      tft.setCursor(145, 80);
      tft.setTextSize(4);
      tft.setTextColor(black, darkgrey);
      tft.print(F("-"));
      tft.setCursor(195, 80);
      tft.print(F("+"));

      tft.setCursor(7, 140);
      tft.setTextColor(black, white);
      tft.setTextSize(2);
      tft.print(F("Volume"));
      tft.fillRect(130, 130, 100, 50, darkgrey);
      tft.drawRect(130, 130, 50, 50, black);
      tft.drawRect(180, 130, 50, 50, black);
      tft.setCursor(145, 140);
      tft.setTextSize(4);
      tft.setTextColor(black, darkgrey);
      tft.print(F("-"));
      tft.setCursor(195, 140);
      tft.print(F("+"));

      tft.drawFastHLine(0, 270, 240, darkgrey);
      tft.setTextSize(2);
      tft.setTextColor(black, white);
      tft.setCursor(68, 290);
      tft.print(F("About..."));

      drawTime(1, darkgrey, true);
      break;
    case PHONE:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 50, red);
      drawTime(1, red, true);
      tft.setTextSize(2);
      tft.setTextColor(black, white);
      tft.setCursor(20, 80);
      tft.print(F("Answer to call"));
      tft.drawFastHLine(0, 120, 240, darkgrey);
      tft.setCursor(20, 140);
      tft.print(F("End call"));
      tft.drawFastHLine(0, 180, 240, darkgrey);
      if (callStat == NOT_CALLING) {
        tft.setCursor(20, 200);
        tft.print(F("Call to number"));
        tft.drawFastHLine(0, 240, 240, darkgrey);
      }
      if (callStat == CALLING_TO) {
        tft.setTextSize(2);
        tft.setCursor(20, 260);
        tft.setTextColor(black, white);
        tft.print(F("Calling to:"));
        tft.setCursor(20, 282);
        tft.print(givenPNumber);
      }
      break;
    case MESSAGES:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 50, darkgreen);
      drawTime(1, darkgreen, true);
      draw(KEYPAD_CHARS, 0);
      tft.fillRect(5, 60, 230, 40, white);
      tft.drawRect(5, 60, 230, 40, black);
      tft.fillRect(5, 110, 160, 40, white);
      tft.drawRect(5, 110, 160, 40, black);
      tft.fillRect(170, 110, 65, 40, green);
      tft.setCursor(175, 130);
      tft.setTextSize(2);
      tft.setTextColor(black, green);
      tft.print(F("Send"));
      break;
    case CONTACTS:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 50, navy);
      drawTime(1, navy, true);
      break;
    /*case PONG:
      tft.fillScreen(black);
      for (int i = 0; i <= 240; i += 2) {
        tft.drawPixel(i, 160, white);
      }
      tft.fillRect(80, 230, 80, 10, white);
      tft.drawFastHLine(0, 5, 240, white);
      tft.setTextColor(white, black);
      tft.setTextSize(2);
      tft.setCursor(0, 20);
      tft.print(F("Use paddle to choose"));
      tft.setCursor(0, 40);
      tft.print(F("level and press OK."));
      tft.setTextSize(4);
      tft.setCursor(15, 200);
      tft.print(F("1   2   3"));
      tft.setTextSize(4);
      tft.setCursor(90, 100);
      tft.print(F("OK"));
      break;*/
  }
  if (a >= 2) {
    lastScreen = a;
  }
  if (a != KEYPAD_NUMBERS && a != KEYPAD_CHARS && b != 0) {
    for (int i = 0; i <= bl; i++) {
      backlight(i);
      delay(10);
    }
  }
}

void drawTime(int a, uint16_t bgColor, bool forceDraw) {
  if (millis() - updateTimer >= 60000 || forceDraw == true) {
    fona.getTime(RTCtime, 23);
    if (a == 0) {
      tft.setCursor(46, 100);
      tft.setTextSize(5);
      tft.setTextColor(cyan, bgColor);
      for (int i = 10; i < 15; i++) {
        tft.print(RTCtime[i]);
      }
    } else {
      tft.setCursor(clockX, clockY);
      tft.setTextSize(4);
      tft.setTextColor(white, bgColor);
      for (int i = 10; i < 15; i++) {
        tft.print(RTCtime[i]);
      }
    }
    updateTimer = millis();
  }
}

void lock() {
  while (ts.touched()) {}
  if (screenDimmed) {
    for (int i = dimmedBL; i >= 0; i--) {
      backlight(i);
      delay(30);
    }
  } else {
    for (int i = bl; i >= 0; i--) {
      backlight(i);
      delay(30);
    }
  }
  phoneLocked = true;
lockBegin:
  tft.fillScreen(blue);
  delay(200);
  if (digitalRead(LOCK_PIN) == 0) {
    draw(LOCKTIME, 0);
    for (int i = 0; i <= bl; i++) {
      backlight(i);
      delay(30);
    }
    while (digitalRead(LOCK_PIN) == 0) {
      if (millis() - updateTimer >= 60000) {
        updateTimer = millis();
        tft.setTextColor(black, cyan);
        tft.setTextSize(6);
        tft.setCursor(35, 20);
        for (int i = 10; i < 15; i++) {
          tft.print(RTCtime[i]);
        }
        tft.setTextColor(white, blue);
        tft.setTextSize(3);
        tft.setCursor(40, 90);
        tft.print(F("BATT:"));
        if (batteryUpdated()) {
          if (getBattery() >= 95) {
            tft.print(F("Full"));
          } else {
            tft.print(F(" "));
            tft.print(getBattery());
            tft.print(F("%"));
          }
        }
      }
      delay(1000);
    }
    for (int i = bl; i >= 0; i--) {
      backlight(i);
      delay(30);
    }
    goto lockBegin;
  } else {
    draw(LOCKSCREEN, 0);
  }
  for (int i = 0; i <= bl; i++) {
    backlight(i);
    delay(30);
  }
  lockscreenOpen();
  givenPassword[0] = ' ';
  givenPassword[1] = ' ';
  givenPassword[2] = ' ';
  givenPassword[3] = ' ';
  if (hasStarted) {
    draw(lastScreen, 1);
  } else {
    draw(MENU, 1);
    hasStarted = true;
  }
}

void backlight(int a) {
  int b = map(a, 0, 20, 0, 255);
  analogWrite(TFT_BL, b);
}

void touchHandler(int a) {
  idleTimer = millis();
  if (screenDimmed) {
    backlight(bl);
    screenDimmed = false;
  } else {
    TS_Point touchPoint = ts.getPoint();
    touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
    touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
    switch (a) {
      case LOCKSCREEN:
        if (touchPoint.x >= 0 && touchPoint.y >= 160 && touchPoint.x <= 75 && touchPoint.y <= 220) {
          insertToGivenPassword(1);
        } else if (touchPoint.x >= 75 && touchPoint.y >= 160 && touchPoint.x <= 120 && touchPoint.y <= 220) {
          insertToGivenPassword(2);
        } else if (touchPoint.x >= 120 && touchPoint.y >= 160 && touchPoint.x <= 185 && touchPoint.y <= 220) {
          insertToGivenPassword(3);
        } else if (touchPoint.x >= 185 && touchPoint.y >= 160 && touchPoint.x <= 240 && touchPoint.y <= 220) {
          insertToGivenPassword(4);
        } else if (touchPoint.x >= 0 && touchPoint.y >= 220 && touchPoint.x <= 75 && touchPoint.y <= 260) {
          insertToGivenPassword(5);
        } else if (touchPoint.x >= 75 && touchPoint.y >= 220 && touchPoint.x <= 120 && touchPoint.y <= 260) {
          insertToGivenPassword(6);
        } else if (touchPoint.x >= 120 && touchPoint.y >= 220 && touchPoint.x <= 186 && touchPoint.y <= 260) {
          insertToGivenPassword(7);
        } else if (touchPoint.x >= 185 && touchPoint.y >= 220 && touchPoint.x <= 240 && touchPoint.y <= 260) {
          insertToGivenPassword(8);
        } else if (touchPoint.x >= 0 && touchPoint.y >= 260 && touchPoint.x <= 75 && touchPoint.y <= 320) {
          insertToGivenPassword(9);
        } else if (touchPoint.x >= 75 && touchPoint.y >= 260 && touchPoint.x <= 120 && touchPoint.y <= 320) {
          insertToGivenPassword(0);
        } else if (touchPoint.x >= 120 && touchPoint.y >= 260 && touchPoint.x <= 240 && touchPoint.y <= 320) {
          insertToGivenPassword(-1);
        } else if (digitalRead(LOCK_PIN) == 0) {
          lock();
        }
        break;
      case MENU:
        if (digitalRead(LOCK_PIN) == 0) {
          lock();
        } else if (touchPoint.x >= 0 && touchPoint.y >= 55 && touchPoint.x <= 170 && touchPoint.y <= 100) {
          settings();
        } else if (touchPoint.x >= 0 && touchPoint.y >= 100 && touchPoint.x <= 170 && touchPoint.y <= 145) {
          phone();
        } else if (touchPoint.x >= 0 && touchPoint.y >= 145 && touchPoint.x <= 170 && touchPoint.y <= 210) {
          messages();
        } else if (touchPoint.x >= 0 && touchPoint.y >= 210 && touchPoint.x <= 170 && touchPoint.y <= 260) {
          //pong();
        }
        break;
    }
  }
  while (ts.touched()) {}
}

void insertToGivenPassword(int a) {
  if (a == -1) {
    if (givenPassword[3] != ' ') {
      givenPassword[3] = ' ';
    } else if (givenPassword[2] != ' ') {
      givenPassword[2] = ' ';
    } else if (givenPassword[1] != ' ') {
      givenPassword[1] = ' ';
    } else if (givenPassword[0] != ' ') {
      givenPassword[0] = ' ';
    }
    tft.fillRect(7, 7, 226, 66, white);
  } else {
    if (givenPassword[0] == ' ') {
      givenPassword[0] = a;
    } else if (givenPassword[1] == ' ') {
      givenPassword[1] = a;
    } else if (givenPassword[2] == ' ') {
      givenPassword[2] = a;
    } else if (givenPassword[3] == ' ') {
      givenPassword[3] = a;
    }
  }
}

bool checkIfPasswordCorrect() {
  if (givenPassword[0] == password[0] && givenPassword[1] == password[1] && givenPassword[2] == password[2] && givenPassword[3] == password[3]) {
    return true;
  } else {
    return false;
  }
}

void slidePage(bool left, uint16_t color) {
  if (left == false) {
    for (int i = 240; i >= 0; i--) {
      tft.drawFastVLine(i, 50, 270, color);
      tft.drawFastVLine((i - 1), 50, 270, black);
      delayMicroseconds(500);
    }
  } else {
    for (int i = 0; i <= 240; i++) {
      tft.drawFastVLine(i, 50, 270, color);
      tft.drawFastVLine((i + 1), 50, 270, black);
      delayMicroseconds(500);
    }
  }
}

void swapAudio() {
  if (audio == FONA_EXTAUDIO) {
    audio = FONA_HEADSETAUDIO;
    fona.setAudio(audio);
  } else {
    audio = FONA_EXTAUDIO;
    fona.setAudio(audio);
  }
}

int getBattery() {
  fona.getBattPercent(&bat);
  return bat;
}

bool batteryUpdated() {
  batry = getBattery();
  if (batry - oldBatry >= 10) {
    oldBatry = batry;
    return true;
  } else {
    return false;
  }
}

void settings() {
  for (int i = bl; i >= 0; i--) {
    backlight(i);
  }
  tft.fillScreen(white);
  tft.fillRect(0, 0, 240, 50, darkgrey);
  draw(SETTINGS, 0);
  for (int i = 0; i <= bl; i++) {
    backlight(i);
  }
  bool exit = false;
  TS_Point touchPoint = ts.getPoint();
  touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
  touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
  while (exit == false) {
    if (ts.touched()) {
      idleTimer = millis();
      if (screenDimmed) {
        backlight(bl);
        screenDimmed = false;
      } else {
        TS_Point touchPoint = ts.getPoint();
        touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
        touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
        if (touchPoint.y <= 50) {
          exit = true;
        } else if (touchPoint.x >= 130 && touchPoint.y >= 70 && touchPoint.x <= 180 && touchPoint.y <= 120) {
          if (bl >= 2) {
            bl = (bl - 2);
            backlight(bl); // 0-20
          }
        } else if (touchPoint.x >= 180 && touchPoint.y >= 70 && touchPoint.x <= 230 && touchPoint.y <= 120) {
          if (bl <= 18) {
            bl = (bl + 2);
            backlight(bl);
          }
        } else if (touchPoint.x >= 130 && touchPoint.y >= 130 && touchPoint.x <= 180 && touchPoint.y <= 180) {
          if (volume >= 10) {
            volume = (volume - 10);
            setAllVolumes(volume);
            fona.playToolkitTone(6, 1000);
          }
        } else if (touchPoint.x >= 180 && touchPoint.y >= 130 && touchPoint.x <= 230 && touchPoint.y <= 180) {
          if (volume <= 90) {
            volume = (volume + 10);
            setAllVolumes(volume);
            fona.playToolkitTone(6, 1000);
          }
        } else if (touchPoint.y >= 270) {
          slidePage(false, cyan);
          about();
          slidePage(true, white);
          draw(SETTINGS, 0);
        }
      }
      while (ts.touched()) {}
    }
    drawTime(1, darkgrey, false);
    if (millis() - idleTimer >= (idleTimeout - 7000)) {
      screenDimmed = true;
      dimmedBL = (bl / 2);
      backlight(dimmedBL);
      if (millis() - idleTimer >= idleTimeout) {
        lock();
        idleTimer = millis();
        screenDimmed = false;
      }
    }
  }
  exitApp();
}

void phone() {
  draw(PHONE, 1);
  bool exit = false;
  bool subExit = false;
  bool willCall;
  TS_Point touchPoint = ts.getPoint();
  touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
  touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
  while (exit == false) {
    if (digitalRead(FONA_RI) == 0) {
      if (callStat != CALLING_TO) {
        callStat = CALL_FROM;
      } else {
        fona.setPWM(2000);
        delay(100);
        fona.setPWM(0);
        delay(100);
        fona.setPWM(2000);
        delay(100);
        fona.setPWM(0);
      }
    }
    if (callStat == CALL_FROM && oldCallStat == NOT_CALLING) { // If calling status changes from NOT_CALLING to CALL_FROM
      fona.incomingCallNumber(incomingCallNumber);
      tft.fillRect(0, 200, 240, 100, white);
      tft.setTextSize(2);
      tft.setCursor(20, 260);
      tft.setTextColor(black, white);
      tft.print(F("Call from:"));
      tft.setCursor(20, 282);
      tft.print(incomingCallNumber);
      oldCallStat = callStat;
    } else if (callStat == NOT_CALLING && oldCallStat == CALLING_TO || callStat == NOT_CALLING && oldCallStat == CALL_FROM) { // If calling status changes to NOT_CALLING
      tft.fillRect(0, 245, 240, 70, white);
      tft.setTextSize(2);
      tft.setTextColor(black, white);
      tft.setCursor(20, 200);
      tft.print(F("Call to number"));
      tft.drawFastHLine(0, 240, 240, darkgrey);
      oldCallStat = callStat;
    } else if (callStat == CALLING_TO && oldCallStat == NOT_CALLING) { // If calling status changes from NOT_CALLING to CALLING_TO
      tft.setTextSize(2);
      tft.setCursor(20, 260);
      tft.setTextColor(black, white);
      tft.print(F("Calling to:"));
      tft.setCursor(20, 282);
      tft.print(givenPNumber);
      oldCallStat = callStat;
    }
    if (ts.touched()) {
      idleTimer = millis();
      if (screenDimmed) {
        backlight(bl);
        screenDimmed = false;
      } else {
        TS_Point touchPoint = ts.getPoint();
        touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
        touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
        if (touchPoint.y <= 50) {
          exit = true;
        } else if (touchPoint.y <= 120 && touchPoint.y >= 50) {
          fona.pickUp();
        } else if (touchPoint.y <= 180) {
          fona.hangUp();
          if (callStat == CALLING_TO || callStat == CALL_FROM) {
            fona.playToolkitTone(5, 1000);
          }
          callStat = NOT_CALLING;
        } else if (touchPoint.y <= 240 && callStat == NOT_CALLING) {
          subExit = false;
          slidePage(false, lightgrey);
          draw(KEYPAD_NUMBERS, 0);
          tft.fillRect(5, 60, 230, 50, white);
          tft.drawRect(5, 60, 230, 50, black);
          tft.fillRect(20, 120, 200, 30, green);
          tft.setTextSize(2);
          tft.setTextColor(black, green);
          tft.setCursor(90, 125);
          tft.print(F("Call"));
          for (byte i = 0; i <= 8; i++) {
            givenPNumber[i] = ' ';
          }
          while (subExit == false) {
            if (ts.touched()) {
              idleTimer = millis();
              if (screenDimmed) {
                backlight(bl);
                screenDimmed = false;
              } else {
                TS_Point touchPoint = ts.getPoint();
                touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
                touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
                if (touchPoint.y <= 50) {
                  subExit = true;
                  willCall = false;
                } else if (touchPoint.x >= 0 && touchPoint.y >= 160 && touchPoint.x <= 75 && touchPoint.y <= 220) {
                  insertToGivenPNumber('1');
                } else if (touchPoint.x >= 75 && touchPoint.y >= 160 && touchPoint.x <= 120 && touchPoint.y <= 220) {
                  insertToGivenPNumber('2');
                } else if (touchPoint.x >= 120 && touchPoint.y >= 160 && touchPoint.x <= 185 && touchPoint.y <= 220) {
                  insertToGivenPNumber('3');
                } else if (touchPoint.x >= 185 && touchPoint.y >= 160 && touchPoint.x <= 240 && touchPoint.y <= 220) {
                  insertToGivenPNumber('4');
                } else if (touchPoint.x >= 0 && touchPoint.y >= 220 && touchPoint.x <= 75 && touchPoint.y <= 260) {
                  insertToGivenPNumber('5');
                } else if (touchPoint.x >= 75 && touchPoint.y >= 220 && touchPoint.x <= 120 && touchPoint.y <= 260) {
                  insertToGivenPNumber('6');
                } else if (touchPoint.x >= 120 && touchPoint.y >= 220 && touchPoint.x <= 186 && touchPoint.y <= 260) {
                  insertToGivenPNumber('7');
                } else if (touchPoint.x >= 185 && touchPoint.y >= 220 && touchPoint.x <= 240 && touchPoint.y <= 260) {
                  insertToGivenPNumber('8');
                } else if (touchPoint.x >= 0 && touchPoint.y >= 260 && touchPoint.x <= 75 && touchPoint.y <= 320) {
                  insertToGivenPNumber('9');
                } else if (touchPoint.x >= 75 && touchPoint.y >= 260 && touchPoint.x <= 120 && touchPoint.y <= 320) {
                  insertToGivenPNumber('0');
                } else if (touchPoint.x >= 120 && touchPoint.y >= 260 && touchPoint.x <= 240 && touchPoint.y <= 320) {
                  insertToGivenPNumber('M');
                } else if (touchPoint.x >= 20 && touchPoint.y >= 120 && touchPoint.x <= 220 && touchPoint.y <= 150) {
                  subExit = true;
                  willCall = true;
                }
                if (touchPoint.y >= 160) {
                  tft.setTextSize(3);
                  tft.setTextColor(black, white);
                  tft.setCursor(35, 75);
                  tft.print(givenPNumber);
                }
              }
              while (ts.touched()) {}
            }
          }
          slidePage(true, white);
          if (willCall == true) {
            fona.callPhone(givenPNumber);
            callStat = CALLING_TO;
          } else {
            tft.setTextSize(2);
            tft.setTextColor(black, white);
            tft.setCursor(20, 200);
            tft.print(F("Call to number"));
            tft.drawFastHLine(0, 240, 240, darkgrey);
          }
          tft.setTextSize(2);
          tft.setTextColor(black, white);
          tft.setCursor(20, 80);
          tft.print(F("Answer to call"));
          tft.drawFastHLine(0, 120, 240, darkgrey);
          tft.setCursor(20, 140);
          tft.print(F("End call"));
          tft.drawFastHLine(0, 180, 240, darkgrey);
        }
      }
    }
    drawTime(1, red, false);
    if (millis() - idleTimer >= (idleTimeout - 7000)) {
      screenDimmed = true;
      dimmedBL = (bl / 2);
      backlight(dimmedBL);
      if (millis() - idleTimer >= idleTimeout) {
        lock();
        idleTimer = millis();
        screenDimmed = false;
      }
    }
  }
  exitApp();
}

void insertToGivenPNumber(char a) {
  if (a == 'M') {
    if (givenPNumber[8] != ' ') {
      givenPNumber[8] = ' ';
    } else if (givenPNumber[7] != ' ') {
      givenPNumber[7] = ' ';
    } else if (givenPNumber[6] != ' ') {
      givenPNumber[6] = ' ';
    } else if (givenPNumber[5] != ' ') {
      givenPNumber[5] = ' ';
    } else if (givenPNumber[4] != ' ') {
      givenPNumber[4] = ' ';
    } else if (givenPNumber[3] != ' ') {
      givenPNumber[3] = ' ';
    } else if (givenPNumber[2] != ' ') {
      givenPNumber[2] = ' ';
    } else if (givenPNumber[1] != ' ') {
      givenPNumber[1] = ' ';
    } else if (givenPNumber[0] != ' ') {
      givenPNumber[0] = ' ';
    }
    tft.fillRect(7, 62, 226, 46, white);
  } else {
    if (givenPNumber[0] == ' ') {
      givenPNumber[0] = a;
    } else if (givenPNumber[1] == ' ') {
      givenPNumber[1] = a;
    } else if (givenPNumber[2] == ' ') {
      givenPNumber[2] = a;
    } else if (givenPNumber[3] == ' ') {
      givenPNumber[3] = a;
    } else if (givenPNumber[4] == ' ') {
      givenPNumber[4] = a;
    } else if (givenPNumber[5] == ' ') {
      givenPNumber[5] = a;
    } else if (givenPNumber[6] == ' ') {
      givenPNumber[6] = a;
    } else if (givenPNumber[7] == ' ') {
      givenPNumber[7] = a;
    } else if (givenPNumber[8] == ' ') {
      givenPNumber[8] = a;
    }
  }
}

void messages() {
  draw(MESSAGES, 1);
  bool exit = false;
  char chars[] = "aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ.!?";
  byte charOnScreenNum = 0;
  char charOnScreen = chars[charOnScreenNum];
  tft.setTextSize(5);
  tft.setTextColor(black, white);
  tft.setCursor(55, 255);
  tft.print(charOnScreen);
  char message[16] = {' '};
  byte messageIndex = 0;
  byte inputState = MESSAGE;
  TS_Point touchPoint = ts.getPoint();
  touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
  touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
  while (exit == false) {
    if (ts.touched()) {
      idleTimer = millis();
      if (screenDimmed) {
        backlight(bl);
        screenDimmed = false;
      } else {
        TS_Point touchPoint = ts.getPoint();
        touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
        touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
        if (touchPoint.y <= 50) {
          exit = true;
        } else if (touchPoint.y <= 100) {
          inputState = MESSAGE;
          draw(KEYPAD_CHARS, 0);
        } else if (touchPoint.y <= 150 && touchPoint.x <= 150) {
          inputState = NUMBER;
          draw(KEYPAD_NUMBERS, 0);
        } else if (touchPoint.y <= 150) {
          messageIndex = 0;
          tft.fillRect(10, 70, 220, 20, white);
          fona.sendSMS(givenPNumber, message);
          for (byte i = 0; i <= 8; i++) {
            givenPNumber[i] = ' ';
          }
          for (byte i = 0; i <= 16; i++) {
            message[i] = ' ';
          }
          tft.setTextSize(2);
          tft.setTextColor(black, white);
          tft.setCursor(10, 120);
          tft.print(givenPNumber);
        }

        if (inputState == MESSAGE) {
          if (touchPoint.x >= 20 && touchPoint.y >= 230 && touchPoint.x <= 80 && touchPoint.y <= 320) {
            if (charOnScreenNum == 0) {
              charOnScreenNum = 54;
            } else {
              charOnScreenNum = (charOnScreenNum - 1);
            }
            charOnScreen = chars[charOnScreenNum];
            tft.setTextSize(5);
            tft.setTextColor(black, white);
            tft.setCursor(55, 255);
            tft.print(charOnScreen);
          } else if (touchPoint.x >= 80 && touchPoint.y >= 230 && touchPoint.x <= 160 && touchPoint.y <= 320) {
            charOnScreenNum = (charOnScreenNum + 1);
            if (charOnScreenNum >= 55) {
              charOnScreenNum = 0;
            }
            charOnScreen = chars[charOnScreenNum];
            tft.setTextSize(5);
            tft.setTextColor(black, white);
            tft.setCursor(55, 255);
            tft.print(charOnScreen);
          } else if (touchPoint.x >= 160 && touchPoint.y >= 240 && touchPoint.x <= 200 && touchPoint.y <= 270) {
            message[messageIndex] = charOnScreen;
            if (messageIndex <= 15) {
              messageIndex = (messageIndex + 1);
              tft.setTextSize(2);
              tft.setTextColor(black, white);
              tft.setCursor(10, 70);
              tft.print(message);
            }
          } else if (touchPoint.x >= 160 && touchPoint.y >= 280 && touchPoint.x <= 320 && touchPoint.y <= 320 && messageIndex >= 1) {
            messageIndex = (messageIndex - 1);
            message[messageIndex] = ' ';
            tft.fillRect(10, 70, 220, 20, white);
            tft.setTextSize(2);
            tft.setTextColor(black, white);
            tft.setCursor(10, 70);
            tft.print(message);
          }
        } else if (inputState == NUMBER) {
          if (touchPoint.x >= 0 && touchPoint.y >= 160 && touchPoint.x <= 75 && touchPoint.y <= 220) {
            insertToGivenPNumber('1');
          } else if (touchPoint.x >= 75 && touchPoint.y >= 160 && touchPoint.x <= 120 && touchPoint.y <= 220) {
            insertToGivenPNumber('2');
          } else if (touchPoint.x >= 120 && touchPoint.y >= 160 && touchPoint.x <= 185 && touchPoint.y <= 220) {
            insertToGivenPNumber('3');
          } else if (touchPoint.x >= 185 && touchPoint.y >= 160 && touchPoint.x <= 240 && touchPoint.y <= 220) {
            insertToGivenPNumber('4');
          } else if (touchPoint.x >= 0 && touchPoint.y >= 220 && touchPoint.x <= 75 && touchPoint.y <= 260) {
            insertToGivenPNumber('5');
          } else if (touchPoint.x >= 75 && touchPoint.y >= 220 && touchPoint.x <= 120 && touchPoint.y <= 260) {
            insertToGivenPNumber('6');
          } else if (touchPoint.x >= 120 && touchPoint.y >= 220 && touchPoint.x <= 186 && touchPoint.y <= 260) {
            insertToGivenPNumber('7');
          } else if (touchPoint.x >= 185 && touchPoint.y >= 220 && touchPoint.x <= 240 && touchPoint.y <= 260) {
            insertToGivenPNumber('8');
          } else if (touchPoint.x >= 0 && touchPoint.y >= 260 && touchPoint.x <= 75 && touchPoint.y <= 320) {
            insertToGivenPNumber('9');
          } else if (touchPoint.x >= 75 && touchPoint.y >= 260 && touchPoint.x <= 120 && touchPoint.y <= 320) {
            insertToGivenPNumber('0');
          } else if (touchPoint.x >= 120 && touchPoint.y >= 260 && touchPoint.x <= 240 && touchPoint.y <= 320) {
            insertToGivenPNumber('M');
          }
          if (touchPoint.y >= 160) {
            tft.setCursor(10, 120);
            tft.setTextSize(2);
            tft.setTextColor(black, white);
            tft.print(givenPNumber);
          }
        }
      }
      while (ts.touched()) {}
    }
    drawTime(1, darkgreen, false);
    if (millis() - idleTimer >= (idleTimeout - 7000)) {
      screenDimmed = true;
      dimmedBL = (bl / 2);
      backlight(dimmedBL);
      if (millis() - idleTimer >= idleTimeout) {
        lock();
        idleTimer = millis();
        screenDimmed = false;
      }
    }
  }
  exitApp();
}

/*void pong() {
  draw(PONG, 1);
  bool exit = false;
  byte ballX = 120;
  byte ballY = 160;
  byte ballXSpeed;
  byte ballYSpeed;
  byte paddleX = 80;
  byte oldPaddleX = paddleX;
  bool gameOn = false;
  TS_Point touchPoint = ts.getPoint();
  touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
  touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
  while (exit == false) {
    if (oldPaddleX != paddleX) {
      tft.fillRect(oldPaddleX, 230, 80, 10, black);
      tft.fillRect(paddleX, 230, 80, 10, white);
      oldPaddleX = paddleX;
    }
    if (gameOn == true) {
      if (ballXSpeed > 0 || ballYSpeed > 0) {
        tft.fillRect(ballX, ballY, 10, 10, white);
      }
      delay(50);
      if (ballXSpeed > 0 || ballYSpeed > 0) {
        tft.fillRect(ballX, ballY, 10, 10, black);
      }
      ballX = (ballX + ballXSpeed);
      ballY = (ballY + ballYSpeed);
      if (ballX <= 3 || ballX >= 230) {
        ballXSpeed = -ballXSpeed;
      }
      if (ballY <= 10) {
        ballYSpeed = -ballYSpeed;
      }
      if (ballY >= 220) {
        if (ballX >= paddleX && ballX <= (paddleX + 80)) {
          ballYSpeed = -ballYSpeed;
          byte toAdd = map(ballX, paddleX, (paddleX + 80), -10, 10);
          ballXSpeed = (ballXSpeed + toAdd);
        }
      }
      if (ballY >= 140 && ballY <= 180) {
        for (int i = 0; i <= 240; i += 2) {
          tft.drawPixel(i, 160, white);
        }
      }
      if (ballY >= 250) {
        ballXSpeed = 0;
        ballYSpeed = 0;
        tft.setCursor(25, 20);
        tft.setTextSize(4);
        tft.setTextColor(white, black);
        tft.print(F("YOU LOSE"));
        delay(2000);
        tft.fillRect(25, 20, 200, 60, black);
        tft.setTextColor(white, black);
        tft.setTextSize(2);
        tft.setCursor(0, 20);
        tft.print(F("Use paddle to choose"));
        tft.setCursor(0, 40);
        tft.print(F("level and press OK."));
        tft.setTextSize(4);
        tft.setCursor(15, 200);
        tft.print(F("1   2   3"));
        tft.setTextSize(4);
        tft.setCursor(90, 100);
        tft.print(F("OK"));
        ballX = 120;
        ballY = 160;
        gameOn = false;
      }
    }
    if (ts.touched()) {
      idleTimer = millis();
      if (screenDimmed) {
        backlight(bl);
        screenDimmed = false;
      } else {
        TS_Point touchPoint = ts.getPoint();
        touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
        touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
        if (gameOn == false && touchPoint.x >= 80 && touchPoint.y >= 100 && touchPoint.x <= 160 && touchPoint.y <= 160) {
          if (paddleX >= 0 && paddleX <= 53) {
            ballXSpeed = 4;
            ballYSpeed = -4;
          } else if (paddleX >= 53 && paddleX <= 106) {
            ballXSpeed = 7;
            ballYSpeed = -7;
          } else if (paddleX >= 106 && paddleX <= 160) {
            ballXSpeed = 10;
            ballYSpeed = -10;
          }
          tft.fillRect(0, 9, 240, 220, black);
          for (int i = 0; i <= 240; i += 2) {
            tft.drawPixel(i, 160, white);
          }
          gameOn = true;
        } else {
          if (touchPoint.x <= 40) {
            paddleX = 0;
          } else {
            paddleX = (touchPoint.x - 40);
          }
          if (paddleX > 160) {
            paddleX = 160;
          }
        }
        if (touchPoint.y <= 50) {
          exit = true;
        }
      }
    }
    if (millis() - idleTimer >= (idleTimeout - 7000)) {
      screenDimmed = true;
      dimmedBL = (bl / 2);
      backlight(dimmedBL);
      if (millis() - idleTimer >= idleTimeout) {
        lock();
        idleTimer = millis();
        screenDimmed = false;
      }
    }
  }
  exitApp();
  }*/

void exitApp() {
  draw(MENU, 1);
}

bool checkNetStat() {
  netStat = fona.getNetworkStatus();
  if (netStat != oldNetStat) {
    return true;
  } else {
    return false;
  }
}

void printNetStat() {
  netStat = fona.getNetworkStatus();
  tft.setTextSize(2);
  tft.setTextColor(white, blue);
  tft.setCursor(20, 13);
  tft.print(netStat);
}

void lockscreenOpen() {
  updateTimer = 60000;
  while (phoneLocked) {
    if (ts.touched()) {
      touchHandler(0);
      tft.setTextSize(5);
      tft.setTextColor(black, white);
      tft.setCursor(LSTextX, LSTextY);
      if (givenPassword[0] != ' ') {
        tft.print(givenPassword[0]);
      }
      if (givenPassword[1] != ' ') {
        tft.print(givenPassword[1]);
      }
      if (givenPassword[2] != ' ') {
        tft.print(givenPassword[2]);
      }
      if (givenPassword[3] != ' ') {
        tft.print(givenPassword[3]);
        phoneLocked = !checkIfPasswordCorrect();
        if (phoneLocked) {
          fona.setPWM(2000);
          givenPassword[0] = ' ';
          givenPassword[1] = ' ';
          givenPassword[2] = ' ';
          givenPassword[3] = ' ';
          tft.fillRect(7, 7, 226, 66, white);
          delay(100);
          fona.setPWM(0);
        }
      }
      while (ts.touched()) {}
    }
    drawTime(0, blue, false);
    if (digitalRead(LOCK_PIN) == 0) {
      lock();
    }
    if (millis() - idleTimer >= (idleTimeout - 7000)) {
      screenDimmed = true;
      dimmedBL = (bl / 3);
      backlight(dimmedBL);
      if (millis() - idleTimer >= idleTimeout) {
        lock();
        idleTimer = millis();
        screenDimmed = false;
      }
    }
    if (digitalRead(LOCK_PIN) == 0) {
      lock();
    }
  }
}

void about() {
  tft.setTextSize(2);
  tft.setTextColor(black, cyan);
  tft.setCursor(0, 70);
  for (int i = 0; i <= 119; i++) {
    char toScreen = EEPROM.read(i);
    if (toScreen == 'Z') {
      toScreen = '\n';
    }
    if (toScreen == '_') {
      toScreen = ' ';
    }
    tft.print(toScreen);
  }
  while (!ts.touched()) {}
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

void setAllVolumes(byte vol) {
  fona.setCallVolume(vol);
  fona.setVolume(vol);
}




