#include <Adafruit_ILI9341.h> // Libraries
#include <Adafruit_FONA.h>
#include <Adafruit_FT6206.h>
#include <SoftwareSerial.h>

Adafruit_FT6206 ts = Adafruit_FT6206(); // Library initializations

#define TFT_CS 10
#define TFT_DC 9
#define SD_CS 4
#define TFT_BL 5
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);


#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST -1
#define FONA_RI 6
#define FONA_KEY 8
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

#define lightgrey ILI9341_LIGHTGREY // System definitions/variables
#define green ILI9341_GREEN
#define black ILI9341_BLACK
#define blue ILI9341_BLUE
#define navy ILI9341_NAVY
#define white ILI9341_WHITE
#define red ILI9341_RED
#define cyan ILI9341_CYAN
#define darkgreen ILI9341_DARKGREEN
#define maroon ILI9341_MAROON
#define greenyellow ILI9341_GREENYELLOW
#define darkgrey ILI9341_DARKGREY

#define LOCKTIME -2
#define KEYPAD -1
#define LOCKSCREEN 0
#define MENU 1
#define SETTINGS 2
#define RADIO 3
#define PHONE 4
#define MESSAGES 5
#define CONTACTS 6
#define PONG 7

#define LSTextX 60
#define LSTextY 20

#define clockX 75
#define clockY 17

#define batteryX 190
#define batteryY 16

char* alphabets[] = {"a", "b", "c", "d", "e", "f", "g"};

int password[] = {1, 2, 3, 4}; // Default password
int givenPassword[] = {NULL, NULL, NULL, NULL};

int bl = 16;
int volume = 1;
int FMvolume = 0;
bool radioState = false;
bool screenDimmed = false;
int dimmedBL;
int audio = FONA_HEADSETAUDIO; // Other audio setting is FONA_HEADSETAUDIO

bool phoneLocked = false;

int screen = 0;
int lastScreen = 1;

int time[] = {2, 30};
long timeTimer = millis();
bool timeUpdated = true;
uint16_t timeBGcolors[] = {blue, blue, darkgrey, black, red, darkgreen, navy,    black, blue};

int numOfAppsP1 = 5;
int numOfAppsP2 = 2;
int appX[] = {20, 20, 20, 20, 20,    60, 60, 60, 60, 60};
int appY[] = {70, 120, 170, 220, 270,    70, 120, 170, 220, 270};
char* appName[] = {"Settings", "Radio", "Phone", "Messages", "Contacts",    "Pong", "Images"};
uint16_t appColor[] = {darkgrey, black, red, darkgreen, navy,    black, blue};
int page = 0;

long idleTimer;
int idleTimeout = 30000;

long uptime = millis();

bool hasStarted = false;

int RchannelNum = 2;
int RchannelX[] = {20, 20, 20, 20, 20};
int RchannelY[] = {70, 110, 150, 190, 230};
char* RchannelName[] = {"RTL", "ELDO"};
int RchannelFrq[] = {889, 1050};
int Rpage = 0;

uint16_t bat;
int batry;
int oldBatry;
long batTimer = millis();

void setup() {
  Serial.begin(9600); // For debugging/testing coordinates
  pinMode(TFT_BL, OUTPUT);
  pinMode(FONA_KEY, OUTPUT);
  digitalWrite(FONA_KEY, HIGH);
  tft.begin();
  tft.setRotation(0);
  tft.setTextSize(7);
  tft.fillScreen(navy);
  tft.setCursor(10, 20);
  tft.setTextColor(green);
  tft.print(F("F"));
  tft.setTextColor(lightgrey);
  tft.print(F("a"));
  tft.setTextColor(red);
  tft.print(F("t"));
  tft.setCursor(65, 80);
  tft.setTextColor(cyan);
  tft.print(F("F"));
  tft.setTextColor(green);
  tft.print(F("o"));
  tft.setTextColor(lightgrey);
  tft.print(F("n"));
  tft.setTextColor(red);
  tft.print(F("e"));
  tft.setTextColor(white);
  tft.setTextSize(3);
  tft.setCursor(50, 170);
  tft.print(F("Starting"));
  tft.setCursor(85, 200);
  for (int i = 0; i <= bl; i++) {
    backlight(i);
    delay(20);
  }
  ts.begin(40);
  fonaSerial->begin(4800);
  tft.print(F("."));
  fona.begin(*fonaSerial);
  tft.print(F("."));
  fona.setAudio(audio);
  tft.print(F("."));
  fona.setVolume(volume);
  tft.print(F("."));
  fona.setFMVolume(FMvolume);
  fona.playToolkitTone(6, 500);
  pinMode(FONA_RI, INPUT);
  tft.setTextWrap(false);
  draw(LOCKSCREEN, 2);
  idleTimer = millis();
  oldBatry = getBattery();
}

void loop() {
  if (phoneLocked) {
    while (phoneLocked) {
      if (ts.touched()) {
        touchHandler(0);
        tft.setTextSize(5);
        tft.setTextColor(black, white);
        tft.setCursor(LSTextX, LSTextY);
        if (givenPassword[0] != NULL) {
          tft.print(givenPassword[0]);
        }
        if (givenPassword[1] != NULL) {
          tft.print(givenPassword[1]);
        }
        if (givenPassword[2] != NULL) {
          tft.print(givenPassword[2]);
        }
        if (givenPassword[3] != NULL) {
          tft.print(givenPassword[3]);
          phoneLocked = !checkIfPasswordCorrect();
          if (phoneLocked) {
            fona.setPWM(2000);
            givenPassword[0] = NULL;
            givenPassword[1] = NULL;
            givenPassword[2] = NULL;
            givenPassword[3] = NULL;
            tft.fillRect(5, 5, 230, 70, white);
            tft.drawRect(5, 5, 230, 70, black);
            delay(100);
            fona.setPWM(0);
          }
        }
        while (ts.touched()) {}
      }
      if (timeUpdated) {
        drawTime(0, blue);
      }
      updateTime();
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
    }
    givenPassword[0] = NULL;
    givenPassword[1] = NULL;
    givenPassword[2] = NULL;
    givenPassword[3] = NULL;
    if (hasStarted) {
      draw(lastScreen, 1);
    } else {
      draw(MENU, 1);
      hasStarted = true;
    }
  }
  if (timeUpdated) {
    drawTime(1, timeBGcolors[screen]);
  }
  if (ts.touched()) {
    touchHandler(screen);
  }
  updateTime();
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
  if (screen == MENU) {
    if (batteryUpdated()) {
      tft.setTextColor(white, blue);
      tft.setTextSize(2);
      tft.setCursor(batteryX, batteryY);
      tft.print(getBattery());
      tft.print(F("%"));
    }
  }
}

void draw(int a, int b) {
  if (a != -1 && b != 0) {
    for (int i = bl; i >= 0; i--) {
      backlight(i);
      delay(10);
    }
    screen = a;
  }
  switch (a) {
    case LOCKTIME:
      tft.setTextColor(white, blue);
      tft.setTextSize(6);
      tft.setCursor(50, 20);
      if (time[0] > 10) {
        tft.setCursor(30, 20);
      }
      tft.print(time[0]);
      tft.print(F(":"));
      tft.print(time[1]);
      tft.setTextColor(white, blue);
      tft.setTextSize(3);
      tft.setCursor(45, 90);
      tft.print(F("BATT. "));
      tft.print(getBattery());
      tft.print(F("%"));
      break;
    case KEYPAD:
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
    case LOCKSCREEN:
      if (b == 2) {
        tft.fillScreen(blue);
      }
      tft.fillRect(5, 5, 230, 70, white);
      tft.drawRect(5, 5, 230, 70, black);
      tft.fillRect(10, 90, 100, 50, maroon);
      tft.drawRect(10, 90, 100, 50, black);
      tft.setCursor(25, 105);
      tft.setTextSize(3);
      tft.setTextColor(white, maroon);
      tft.print(F("LOCK"));
      draw(KEYPAD, 0);
      drawTime(0, blue);
      tft.setTextSize(5);
      tft.setTextColor(black, white);
      tft.setCursor(LSTextX, LSTextY);
      if (givenPassword[0] != NULL) {
        tft.print(givenPassword[0]);
      }
      if (givenPassword[1] != NULL) {
        tft.print(givenPassword[1]);
      }
      if (givenPassword[2] != NULL) {
        tft.print(givenPassword[2]);
      }
      phoneLocked = true;
      break;
    case MENU:
      tft.fillScreen(cyan);
      tft.fillRect(0, 0, 240, 50, blue);
      if (page == 0) {
        for (int i = 0; i < numOfAppsP1; i++) {
          tft.setCursor(appX[i], appY[i]);
          tft.setTextColor(appColor[i], cyan);
          tft.setTextSize(3);
          tft.print(appName[i]);
        }
        tft.fillTriangle(200, 200, 200, 120, 230, 160, green);
        tft.drawTriangle(200, 200, 200, 120, 230, 160, black);
      } else {
        for (int i = 5; (i - 5) < numOfAppsP2; i++) {
          tft.setCursor(appX[i], appY[i]);
          tft.setTextColor(appColor[i], cyan);
          tft.setTextSize(3);
          tft.print(appName[i]);
        }
        tft.fillTriangle(40, 200, 40, 120, 10, 160, green);
        tft.drawTriangle(40, 200, 40, 120, 10, 160, black);
      }
      tft.fillRect(5, 5, 60, 35, maroon);
      tft.drawRect(5, 5, 60, 35, black);
      tft.setTextSize(2);
      tft.setTextColor(white, maroon);
      tft.setCursor(13, 17);
      tft.print(F("LOCK"));
      drawTime(1, blue);
      lastScreen = MENU;
      tft.setTextColor(white, blue);
      tft.setTextSize(2);
      tft.setCursor(batteryX, batteryY);
      tft.print(getBattery());
      tft.print(F("%"));
      break;
    case SETTINGS:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 50, darkgrey);
      drawTime(1, darkgrey);
      break;
    case RADIO:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 50, black);
      drawTime(1, black);
      tft.setTextColor(black, white);
      tft.setTextSize(3);
      for (int i = 0; i < RchannelNum; i++) {
        tft.setCursor(RchannelX[i], RchannelY[i]);
        tft.print(RchannelName[i]);
      }
      tft.drawFastHLine(0, 250, 240, black);
      tft.drawCircle(120, 280, 20, black);
      tft.drawFastHLine(110, 280, 20, black);
      tft.drawFastVLine(120, 270, 20, black);
      break;
    case PHONE:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 50, red);
      drawTime(1, red);
      break;
    case MESSAGES:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 50, darkgreen);
      drawTime(1, darkgreen);
      break;
    case CONTACTS:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 50, navy);
      drawTime(1, navy);
      break;
    case PONG:
      tft.fillScreen(black);
      for (int i = 0; i <= 240; i += 2) {
        tft.drawPixel(i, 160, white);
      }
      tft.fillRect(110, 300, 130, 310, white);
      break;
  }
  if (a >= 2) {
    lastScreen = a;
  }
  if (a != -1 && b != 0) {
    for (int i = 0; i <= bl; i++) {
      backlight(i);
      delay(10);
    }
  }
}

void drawTime(int a, uint16_t bgColor) {
  if (a == 0) {
    tft.setCursor(130, 105);
    tft.setTextSize(3);
    tft.setTextColor(white, bgColor);
    if (time[0] < 10) {
      tft.setCursor(137, 105);
    }
    tft.print(time[0]);
    tft.print(F(":"));
    if (time[1] < 10) {
      tft.print(F("0"));
    }
    tft.print(time[1]);
  } else {
    tft.setCursor(clockX, clockY);
    tft.setTextSize(3);
    tft.setTextColor(white, bgColor);
    if (time[0] < 10) {
      tft.setCursor((clockX + 7), clockY);
    }
    tft.print(time[0]);
    tft.print(F(":"));
    if (time[1] < 10) {
      tft.print(F("0"));
    }
    tft.print(time[1]);
  }
}

void updateTime() {
  if (millis() - timeTimer >= 60000) {
    time[1] += 1;
    if (time[1] >= 60) {
      time[1] = 0;
      time[0] += 1;
    }
    if (time[0] >= 24) {
      time[0] = 0;
    }
    timeUpdated = true;
    timeTimer = millis();
  } else {
    timeUpdated = false;
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
lockBegin:
  tft.fillScreen(blue);

  while (!ts.touched()) {
    updateTime();
  }
  delay(200);
  if (ts.touched()) {
    draw(LOCKTIME, 0);
    for (int i = 0; i <= bl; i++) {
      backlight(i);
      delay(30);
    }
    while (ts.touched()) {
      updateTime();
      if (timeUpdated) {
        tft.setTextColor(white, blue);
        tft.setTextSize(6);
        tft.setCursor(50, 20);
        if (time[0] > 10) {
          tft.setCursor(30, 20);
        }
        tft.print(time[0]);
        tft.print(F(":"));
        tft.print(time[1]);
        if (batteryUpdated()) {
          tft.setTextColor(white, blue);
          tft.setTextSize(3);
          tft.setCursor(45, 90);
          tft.print(F("BATT. "));
          tft.print(getBattery());
          tft.print(F("%"));
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
    Serial.print(F("X: "));
    Serial.print(touchPoint.x);
    Serial.print(F(", Y: "));
    Serial.println(touchPoint.y);
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
        } else if (touchPoint.x >= 10 && touchPoint.y >= 95 && touchPoint.x <= 120 && touchPoint.y <= 135) {
          lock();
        }
        break;
      case MENU:
        if (page == 0) {
          if (touchPoint.x >= 5 && touchPoint.y >= 5 && touchPoint.x <= 70 && touchPoint.y <= 50) {
            lock();
          } else if (touchPoint.x >= 0 && touchPoint.y >= 55 && touchPoint.x <= 170 && touchPoint.y <= 100) {
            draw(SETTINGS, 1);
          } else if (touchPoint.x >= 0 && touchPoint.y >= 100 && touchPoint.x <= 170 && touchPoint.y <= 145) {
            draw(RADIO, 1);
          } else if (touchPoint.x >= 0 && touchPoint.y >= 145 && touchPoint.x <= 170 && touchPoint.y <= 210) {
            draw(PHONE, 1);
          } else if (touchPoint.x >= 0 && touchPoint.y >= 210 && touchPoint.x <= 170 && touchPoint.y <= 260) {
            draw(MESSAGES, 1);
          } else if (touchPoint.x >= 0 && touchPoint.y >= 260 && touchPoint.x <= 170 && touchPoint.y <= 320) {
            draw(CONTACTS, 1);
          } else if (touchPoint.x >= 180 && touchPoint.y >= 120 && touchPoint.x <= 240 && touchPoint.y <= 190) {
            slidePage(false, cyan);
            page = 1;
            for (int i = 5; (i - 5) < numOfAppsP2; i++) {
              tft.setCursor(appX[i], appY[i]);
              tft.setTextColor(appColor[i], cyan);
              tft.setTextSize(3);
              tft.print(appName[i]);
            }
            tft.fillTriangle(40, 200, 40, 120, 10, 160, green);
            tft.drawTriangle(40, 200, 40, 120, 10, 160, black);
          }
        } else {
          if (touchPoint.x >= 5 && touchPoint.y >= 5 && touchPoint.x <= 70 && touchPoint.y <= 50) {
            lock();
          } else if (touchPoint.x >= 30 && touchPoint.y >= 55 && touchPoint.x <= 240 && touchPoint.y <= 100) {
            draw(PONG, 1);
          } else if (touchPoint.x >= 0 && touchPoint.y >= 120 && touchPoint.x <= 60 && touchPoint.y <= 190) {
            slidePage(true, cyan);
            page = 0;
            for (int i = 0; i < numOfAppsP1; i++) {
              tft.setCursor(appX[i], appY[i]);
              tft.setTextColor(appColor[i], cyan);
              tft.setTextSize(3);
              tft.print(appName[i]);
            }
            tft.fillTriangle(200, 200, 200, 120, 230, 160, green);
            tft.drawTriangle(200, 200, 200, 120, 230, 160, black);
          }
        }
        break;
      case SETTINGS:
        if (touchPoint.y <= 55) {
          draw(MENU, 1);
        }
        break;
      case RADIO:
        if (Rpage == 0) {

        }
        if (touchPoint.y <= 55) {
          draw(MENU, 1);
        } else if (touchPoint.y >= 55 && touchPoint.y <= 105) {
          if (radioState == false) {
            fona.FMradio(true, audio);
            fona.tuneFMradio(RchannelFrq[0]);
            radioState = true;
          } else {
            fona.FMradio(false);
            radioState = false;
          }
        } else if (touchPoint.y >= 105 && touchPoint.y <= 155) {
          if (radioState == false) {
            fona.FMradio(true, audio);
            fona.tuneFMradio(RchannelFrq[1]);
            radioState = true;
          } else {
            fona.FMradio(false);
            radioState = false;
          }
        } else if (touchPoint.y >= 155 && touchPoint.y <= 205) {
          if (radioState == false) {
            fona.FMradio(true, audio);
            fona.tuneFMradio(RchannelFrq[2]);
            radioState = true;
          } else {
            fona.FMradio(false);
            radioState = false;
          }
        } else if (touchPoint.y >= 205 && touchPoint.y <= 255) {
          if (radioState == false) {
            fona.FMradio(true, audio);
            fona.tuneFMradio(RchannelFrq[3]);
            radioState = true;
          } else {
            fona.FMradio(false);
            radioState = false;
          }
        } else if (touchPoint.y >= 255) {
          slidePage(false, white);
        }
        break;
      case PHONE:
        if (touchPoint.y <= 55) {
          draw(MENU, 1);
        }
        break;
      case MESSAGES:
        if (touchPoint.y <= 55) {
          draw(MENU, 1);
        }
        break;
      case CONTACTS:
        if (touchPoint.y <= 55) {
          draw(MENU, 1);
        }
        break;
      case PONG:
        if (touchPoint.y <= 55) {
          draw(MENU, 1);
        }
        break;
    }
  }
  while (ts.touched()) {}
}

void insertToGivenPassword(int a) {
  if (a == -1) {
    if (givenPassword[3] != NULL) {
      givenPassword[3] = NULL;
      tft.fillRect(5, 5, 230, 70, white);
      tft.drawRect(5, 5, 230, 70, black);
    } else if (givenPassword[2] != NULL) {
      givenPassword[2] = NULL;
      tft.fillRect(5, 5, 230, 70, white);
      tft.drawRect(5, 5, 230, 70, black);
    } else if (givenPassword[1] != NULL) {
      givenPassword[1] = NULL;
      tft.fillRect(5, 5, 230, 70, white);
      tft.drawRect(5, 5, 230, 70, black);
    } else if (givenPassword[0] != NULL) {
      givenPassword[0] = NULL;
      tft.fillRect(5, 5, 230, 70, white);
      tft.drawRect(5, 5, 230, 70, black);
    }
  } else {
    if (givenPassword[0] == NULL) {
      givenPassword[0] = a;
    } else if (givenPassword[1] == NULL) {
      givenPassword[1] = a;
    } else if (givenPassword[2] == NULL) {
      givenPassword[2] = a;
    } else if (givenPassword[3] == NULL) {
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
  if (millis() - batTimer >= 10000) {
    batTimer = millis();
    batry = getBattery();
    if (oldBatry != batry) {
      oldBatry = batry;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}






