#include <Adafruit_ILI9341.h> // Libraries
#include <Adafruit_GFX.h>
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

#define KEYPAD -1
#define LOCKSCREEN 0
#define MENU 1
#define SETTINGS 2
#define RADIO 3
#define PHONE 4
#define MESSAGES 5
#define IMAGES 6
#define PONG 7

#define LSTextX 60
#define LSTextY 20

#define clockX 75
#define clockY 17

int password[] = {1, 2, 3, 4}; // Default password
int givenPassword[] = {NULL, NULL, NULL, NULL};

int bl = 16;
int volume = 0;
int FMvolume = 0;
bool radioState = false;
bool screenDimmed = false;
int dimmedBL;
int audio = FONA_EXTAUDIO; // Other audio setting is FONA_HEADSETAUDIO

bool phoneLocked = false;

int screen = 0;
int lastScreen = 1;

int time[] = {12, 30};
long timeTimer = millis();
bool timeUpdated = true;
uint16_t timeBGcolors[] = {blue, blue, darkgrey, black, red, darkgreen, navy,    black};

int numOfAppsP1 = 5;
int numOfAppsP2 = 1;
int appX[] = {20, 20, 20, 20, 20,    60, 60, 60, 60, 60};
int appY[] = {70, 120, 170, 220, 270,    70, 120, 170, 220, 270};
char* appName[] = {"Settings", "Radio", "Phone", "Messages", "Images",    "Pong"};
uint16_t appColor[] = {darkgrey, black, red, darkgreen, navy,    black};
int page = 0;

long idleTimer;
int idleTimeout = 30000;

long uptime = millis();

bool hasStarted = false;

void setup() {
  Serial.begin(9600); // For debugging/testing coordinates
  pinMode(TFT_BL, OUTPUT);
  tft.begin();
  tft.setRotation(0);
  tft.setTextSize(7);
  tft.fillScreen(navy);
  tft.setCursor(40, 50);
  tft.setTextColor(green);
  tft.print("N");
  tft.setTextColor(lightgrey);
  tft.print("A");
  tft.setTextColor(red);
  tft.print("N");
  tft.setTextColor(cyan);
  tft.print("O");
  tft.setTextColor(white);
  tft.setTextSize(3);
  tft.setCursor(50, 150);
  tft.print("Starting");
  tft.setCursor(85, 180);
  for (int i = 0; i <= bl; i++) {
    backlight(i);
    delay(20);
  }
  ts.begin(40);
  fonaSerial->begin(4800);
  tft.print(".");
  fona.begin(*fonaSerial);
  tft.print(".");
  fona.setAudio(audio);
  tft.print(".");
  fona.setVolume(volume);
  tft.print(".");
  fona.setFMVolume(FMvolume);
  fona.playToolkitTone(6, 500);
  pinMode(FONA_RI, INPUT);
  pinMode(FONA_KEY, OUTPUT);
  tft.setTextWrap(false);
  draw(LOCKSCREEN, 1);
  idleTimer = millis();
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
        dimmedBL = (bl / 2);
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
}

void draw(int a, int b) {
  if (b == 0) {} else if (a != -1) {
    for (int i = bl; i >= 0; i--) {
      backlight(i);
      delay(10);
    }
    screen = a;
  }
  switch (a) {
    case KEYPAD:
      tft.fillRect(0, 160, 240, 320, white);
      tft.drawFastHLine(0, 159, 240, black);
      tft.drawFastHLine(0, 160, 240, black);

      tft.setTextSize(5);
      tft.setTextColor(black, white);
      tft.setCursor(20, 170);
      tft.print("1 2 3 4");
      tft.setCursor(20, 220);
      tft.print("5 6 7 8");
      tft.setCursor(20, 270);
      tft.print("9 0 <<");

      break;
    case LOCKSCREEN:
      tft.fillScreen(blue);
      tft.fillRect(5, 5, 230, 70, white);
      tft.drawRect(5, 5, 230, 70, black);
      tft.fillRect(10, 90, 100, 50, maroon);
      tft.drawRect(10, 90, 100, 50, black);
      tft.setCursor(25, 105);
      tft.setTextSize(3);
      tft.setTextColor(white, maroon);
      tft.print("LOCK");
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
      tft.print("LOCK");
      drawTime(1, blue);
      break;
    case SETTINGS:
      tft.fillScreen(lightgrey);
      tft.fillRect(0, 0, 240, 50, darkgrey);
      drawTime(1, darkgrey);
      break;
    case RADIO:
      tft.fillScreen(darkgrey);
      tft.fillRect(0, 0, 240, 50, black);
      drawTime(1, black);
      tft.setCursor(5, 60);
      tft.setTextColor(black);
      tft.setTextSize(2);
      tft.print("Touch to play/pause");
      tft.setCursor(5, 80);
      tft.print("at 88.9 FM");
      break;
    case PHONE:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 50, red);
      drawTime(1, red);
      break;
    case MESSAGES:
      tft.fillScreen(green);
      tft.fillRect(0, 0, 240, 50, darkgreen);
      drawTime(1, darkgreen);
      break;
    case IMAGES:
      tft.fillScreen(blue);
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
  if (b == 0) {} else if (a != -1) {
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
      tft.print("0");
    }
    tft.print(time[0]);
    tft.print(":");
    if (time[1] < 10) {
      tft.print("0");
    }
    tft.print(time[1]);
  } else {
    tft.setCursor(clockX, clockY);
    tft.setTextSize(3);
    tft.setTextColor(white, bgColor);
    if (time[0] < 10) {
      tft.print("0");
    }
    tft.print(time[0]);
    tft.print(":");
    if (time[1] < 10) {
      tft.print("0");
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
  draw(LOCKSCREEN, 0);

  while (ts.touched()) {}
  while (!ts.touched()) {
    updateTime();
  }
  while (ts.touched()) {}
  drawTime(0, blue);
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
    Serial.print("X: ");
    Serial.print(touchPoint.x);
    Serial.print(", Y: ");
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
            draw(IMAGES, 1);
          } else if (touchPoint.x >= 180 && touchPoint.y >= 120 && touchPoint.x <= 240 && touchPoint.y <= 190) {
            slidePage();
          }
        } else {
          if (touchPoint.x >= 5 && touchPoint.y >= 5 && touchPoint.x <= 70 && touchPoint.y <= 50) {
            lock();
          } else if (touchPoint.x >= 30 && touchPoint.y >= 55 && touchPoint.x <= 240 && touchPoint.y <= 100) {
            draw(PONG, 1);
          } else if (touchPoint.x >= 0 && touchPoint.y >= 120 && touchPoint.x <= 60 && touchPoint.y <= 190) {
            slidePage();
          }
        }
        break;
      case SETTINGS:
        if (touchPoint.y <= 55) {
          draw(MENU, 1);
        }
        break;
      case RADIO:
        if (touchPoint.y <= 55) {
          draw(MENU, 1);
        } else if (touchPoint.y >= 55) {
          if (radioState == true) {
            fona.FMradio(true, audio);
            fona.tuneFMradio(889);
            radioState = false;
          } else {
            fona.FMradio(false);
            radioState = true;
          }
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
      case IMAGES:
        if (touchPoint.y <= 55) {
          draw(MENU, 1);
        }
        break;
      case PONG:
        if (touchPoint.y <= 55 && touchPoint.x <= 100) {
          draw(MENU, 1);
        }
        break;
    }
  }
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
  } else if (givenPassword[0] == 3 && givenPassword[1] == 3 && givenPassword[2] == 3 && givenPassword[3] == 3) {
    delay(10000);
    tft.fillTriangle(120, 90, 20, 240, 220, 240, darkgreen);
    return true;
  } else {
    return false;
  }
}

void slidePage() {
  if (page == 0) {
    page = 1;
    for (int i = 240; i >= 0; i--) {
      tft.drawFastVLine(i, 50, 270, cyan);
      tft.drawFastVLine((i - 1), 50, 270, black);
      delay(1);
    }
    for (int i = 5; (i - 5) < numOfAppsP2; i++) {
      tft.setCursor(appX[i], appY[i]);
      tft.setTextColor(appColor[i], cyan);
      tft.setTextSize(3);
      tft.print(appName[i]);
    }
    tft.fillTriangle(40, 200, 40, 120, 10, 160, green);
    tft.drawTriangle(40, 200, 40, 120, 10, 160, black);
  } else {
    page = 0;
    for (int i = 0; i <= 240; i++) {
      tft.drawFastVLine(i, 50, 270, cyan);
      tft.drawFastVLine((i + 1), 50, 270, black);
      delay(1);
    }
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

void swapAudio() {
  if (audio == FONA_EXTAUDIO) {
    audio = FONA_HEADSETAUDIO;
    fona.setAudio(audio);
  } else {
    audio = FONA_EXTAUDIO;
    fona.setAudio(audio);
  }
}






