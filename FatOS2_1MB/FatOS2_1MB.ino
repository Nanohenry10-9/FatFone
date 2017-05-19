#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <Adafruit_FONA.h>
#include <SoftwareSerial.h>
#include <SD.h>

Adafruit_FT6206 ts = Adafruit_FT6206();

#define TFT_CS 10
#define TFT_DC 9
#define TFT_BL 5
#define SD_CS 4
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define FONA_RX 2
#define FONA_TX 12
#define FONA_RST 4
#define FONA_RI 3
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
#define orange        ILI9341_ORANGE
#define olive         ILI9341_OLIVE

#define SCREEN_HOME   0
#define SCREEN_MENU   1
#define SCREEN_LOCK   2
#define SCREEN_PHONE  3
#define SCREEN_SMS1   4
#define SCREEN_SMS2   5
#define SCREEN_SET    6
#define SCREEN_CONT   7
#define SCREEN_RADIO  8
#define SCREEN_CALC   9
#define SCREEN_PONG   10

#define NUMPAD_W  140
#define NUMPAD_H  190

int bl = 100;

char* appNames[] = {"Phone", "SMS", "Set.", "Cont.", "PONG", "Radio", "Calc", "..."};
int appColor[] = {red, green, darkgrey, blue, black, navy, orange, olive};
//int appColor[] = {black, black, black, black, black, black, black, black};
byte numOfApps = 8;
#define appHeight 64 // Screen height (320) / 5

int tX = 0;
int tY = 0;
int oldX;
int oldY;
TS_Point TP;

int i = 0;
int a = 0;

byte openApp = SCREEN_MENU;

char kpC[] = {'1', '2', '3',
              '4', '5', '6',
              '7', '8', '9',
              '+', '0', '#'
             };

byte kpX[] = {18, 61, 104,
              18, 61, 104,
              18, 61, 104,
              18, 61, 104
             };

byte kpY[] = {20, 20, 20,
              65, 65, 65,
              108, 108, 108,
              150, 150, 150
             };

char time[] = {"00:00"};
uint16_t batt = 100;

long updateTimer = 0;

char phoneNumber[23] = {' '};
char incNumber[23] = {' '};
int phoneNumberIndex = 0;

int numpadX = 0;
int numpadY = 0;

char psword[] = {"1234"};
char inputpw[] = {"    "};
int inputpwindex = 0;

char* settings[4] = {"Battery", "Screen", "Sounds", "Phone"};

void setup() {
  Serial.begin(9600);
  pinMode(18, INPUT_PULLUP);
  tft.begin();
  analogWrite(TFT_BL, bl);
  tft.fillScreen(navy);
  tft.setTextSize(1);
  tft.setTextColor(white, navy);
  tft.print(F("Starting touchscreen... "));
  ts.begin();
  tft.println(F("Done"));
  tft.print(F("Initializing SD library... "));
  SD.begin(SD_CS);
  tft.println(F("Done"));
  tft.print(F("Starting FONA [1/2]... "));
  fonaSerial->begin(4800);
  tft.println(F("Done"));
  tft.print(F("Starting FONA [2/2]... "));
  fona.begin(*fonaSerial);
  tft.println(F("Done"));
  tft.print(F("Setting FONA settings... "));
  fona.setAudio(FONA_EXTAUDIO);
  fona.setAllVolumes(50);
  fona.setPWM(0);
  tft.println(F("Done"));
  tft.print(F("Starting graphical UI..."));
  draw(SCREEN_LOCK, 0);
  //tone(45, 600, 30);
  //delay(40);
  //tone(45, 600, 200);
}

void loop() {
  if (ts.touched()) {
    touchHandler(openApp);
  }
  if (millis() - updateTimer >= 10000) {
    appRoutine(openApp);
    Serial.println("Update time!");
    updateTimer = millis();
  }
  if (homeBtn() && openApp != SCREEN_MENU && openApp != SCREEN_LOCK) {
    draw(SCREEN_MENU, true);
  }
  if (fona.incomingCallNumber(incNumber) || !digitalRead(FONA_RI)) {
    Serial.println("Incoming call");
    callFrom();
  }
}

void touchHandler(byte screen) {
  tX = getX();
  tY = getY();
  Serial.print("TS touched [X, Y]: ");
  Serial.print(tX);
  Serial.print(", ");
  Serial.println(tY);
  switch (screen) {
    case SCREEN_HOME:
      if (tX >= 30 && tX <= 210 && tY >= 190 && tY <= 290) {
        drawButton(30, 190, 180, 100, "Open Menu", 3, darkgrey, lightgrey, true, false);
        while (ts.touched());
        draw(SCREEN_MENU, true);
      }
      break;
    case SCREEN_MENU:
      if (tX < 120) {
        if (tY < appHeight * 2 && tY > appHeight) {
          drawTRect(0, appHeight, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_PHONE, 1);
        } else if (tY < appHeight * 3 && tY > appHeight * 2) {
          drawTRect(0, appHeight * 2, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_SET, 1);
        } else if (tY < appHeight * 4 && tY > appHeight * 3) {
          drawTRect(0, appHeight * 3, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_PONG, 1);
        } else if (tY > appHeight * 4) {
          drawTRect(0, appHeight * 4, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_CALC, 1);
        }
      } else {
        if (tY < appHeight * 2 && tY > appHeight) {
          drawTRect(120, appHeight, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_SMS1, 1);
        } else if (tY < appHeight * 3 && tY > appHeight * 2) {
          drawTRect(120, appHeight * 2, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_CONT, 1);
        } else if (tY < appHeight * 4 && tY > appHeight * 3) {
          drawTRect(120, appHeight * 3, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_RADIO, 1);
        } else if (tY > appHeight * 4) {
          drawTRect(120, appHeight * 4, 120, appHeight, navy, 5);
          while (ts.touched());
          //draw(SCREEN_CALC, 1);
        }
      }
      break;
    case SCREEN_LOCK:
      if (tX <= NUMPAD_W && tX >= 0 && tY >= 130 && tY <= 319) {
        int keyPressed = getNumpad();
        tft.setTextColor(white, lightgrey);
        tft.setTextSize(3);
        tft.setCursor(kpX[keyPressed], 130 + kpY[keyPressed]);
        tft.print(kpC[keyPressed]);
        if (kpC[keyPressed] != '+' && kpC[keyPressed] != '#' && inputpwindex < 4) {
          inputpw[inputpwindex] = kpC[keyPressed];
          inputpwindex++;
          tft.setCursor(getTextCenter(inputpw, 120, 4), getTextCenter(65, 4));
          tft.setTextColor(black, white);
          tft.setTextSize(4);
          tft.print(inputpw);
        }
        buzz(5);
        while (ts.touched());
        tft.setTextColor(black, lightgrey);
        tft.setTextSize(3);
        tft.setCursor(kpX[keyPressed], 130 + kpY[keyPressed]);
        tft.print(kpC[keyPressed]);
      }
      if (tX > NUMPAD_W && tY >= 130 && tY <= 225) {
        drawButton(NUMPAD_W, 130, 100, 95, "OK", 4, darkgreen, green, true, false);
        if (inputpw[0] == psword[0] && inputpw[1] == psword[1] && inputpw[2] == psword[2] && inputpw[3] == psword[3]) {
          for (i = 0; i < 4; i++) {
            inputpw[i] = ' ';
            inputpwindex = 0;
          }
          draw(SCREEN_HOME, true);
        } else {
          Serial.print("'");
          Serial.print(inputpw);
          Serial.print("' doesn't match password (");
          Serial.print(psword);
          Serial.println(")");
          tft.setCursor(getTextCenter(inputpw, 120, 4), getTextCenter(65, 4));
          tft.setTextColor(red, white);
          tft.setTextSize(4);
          tft.print(inputpw);
          delay(1000);
          tft.setTextColor(black, white);
          tft.setCursor(getTextCenter(inputpw, 120, 4), getTextCenter(65, 4));
          tft.print(inputpw);
          while (ts.touched());
          drawButton(NUMPAD_W, 130, 100, 95, "OK", 4, darkgreen, green, false, false);
        }
      }
      if (tX > NUMPAD_W && tY > 225) {
        drawButton(NUMPAD_W, 225, 100, 95, "RMV", 4, maroon, red, true, false);
        if (inputpwindex > 0) {
          inputpwindex--;
          inputpw[inputpwindex] = ' ';
          tft.setCursor(getTextCenter(inputpw, 120, 4), getTextCenter(65, 4));
          tft.setTextColor(black, white);
          tft.setTextSize(4);
          tft.print(inputpw);
        }
        while (ts.touched());
        drawButton(NUMPAD_W, 225, 100, 95, "RMV", 4, maroon, red, false, false);
      }

      break;
    case SCREEN_PHONE:
      if (tX >= 10 && tY >= 30 && tX <= 220 && tY <= 120) {
        tft.fillRect(20, 20, 200, 250, white);
        drawTRect(20, 20, 200, 250, black, 3);
        tft.setCursor(36, 40);
        tft.setTextSize(4);
        tft.setTextColor(black, white);
        for (i = 0; i < 7; i++) {
          tft.print(phoneNumber[i]);
        }
        tft.setCursor(36, 100);
        for (i = 7; i < 14; i++) {
          tft.print(phoneNumber[i]);
        }
        tft.setCursor(36, 160);
        for (i = 14; i < 21; i++) {
          tft.print(phoneNumber[i]);
        }
        tft.setCursor(36, 220);
        tft.print(phoneNumber[21]);
        while (ts.touched());
        draw(SCREEN_PHONE, false);
      }
      if (tX >= NUMPAD_W && tX <= NUMPAD_W + 100 && tY >= 130 && tY <= 193) {
        drawButton(NUMPAD_W, 130, 100, 63, "CALL", 3, darkgreen, green, true, false);
        while (ts.touched());
        callTo(phoneNumber);
      }

      if (tX >= NUMPAD_W && tX <= NUMPAD_W + 100 && tY > 193 && tY <= 257) {
        drawButton(NUMPAD_W, 193, 100, 64, "RMV", 3, maroon, red, true, false);
        if (phoneNumberIndex > 0) {
          phoneNumberIndex--;
          phoneNumber[phoneNumberIndex] = ' ';
        }
        while (ts.touched());
        drawButton(NUMPAD_W, 193, 100, 64, "RMV", 3, maroon, red, false, false);
        tft.setCursor(20, 40);
        tft.setTextSize(3);
        tft.setTextColor(black, white);
        for (i = 0; i < 11; i++) {
          tft.print(phoneNumber[i]);
        }
        tft.setCursor(20, 70);
        for (i = 11; i < 22; i++) {
          tft.print(phoneNumber[i]);
        }
      }

      if (tX >= NUMPAD_W && tX <= NUMPAD_W + 100 && tY > 257 && tY < 319) {
        drawButton(NUMPAD_W, 257, 100, 63, "CLEAR", 3, darkgrey, lightgrey, true, false);
        for (i = 0; i < 30; i++) {
          phoneNumber[i] = ' ';
          phoneNumberIndex = 0;
        }
        while (ts.touched());
        drawButton(NUMPAD_W, 257, 100, 63, "CLEAR", 3, darkgrey, lightgrey, false, false);
        tft.setCursor(20, 40);
        tft.setTextSize(3);
        tft.setTextColor(black, white);
        for (i = 0; i < 11; i++) {
          tft.print(phoneNumber[i]);
        }
        tft.setCursor(20, 70);
        for (i = 11; i < 22; i++) {
          tft.print(phoneNumber[i]);
        }
      }

      if (tX <= NUMPAD_W && tX >= 0 && tY >= 130 && tY <= 319) {
        int keyPressed = getNumpad();
        tft.setTextColor(white, lightgrey);
        tft.setTextSize(3);
        tft.setCursor(kpX[keyPressed], 130 + kpY[keyPressed]);
        tft.print(kpC[keyPressed]);
        if (phoneNumberIndex < 22) {
          phoneNumber[phoneNumberIndex] = kpC[keyPressed];
          phoneNumberIndex++;
        }
        buzz(5);
        while (ts.touched());
        tft.setTextColor(black, lightgrey);
        tft.setCursor(kpX[keyPressed], 130 + kpY[keyPressed]);
        tft.print(kpC[keyPressed]);
        tft.setCursor(20, 40);
        tft.setTextSize(3);
        tft.setTextColor(black, white);
        for (i = 0; i < 11; i++) {
          tft.print(phoneNumber[i]);
        }
        tft.setCursor(20, 70);
        for (i = 11; i < 22; i++) {
          tft.print(phoneNumber[i]);
        }
      }
      break;
    case SCREEN_SMS1:
      break;
    case SCREEN_SMS2:
      break;
    case SCREEN_SET:
      break;
  }
}

void callTo(char tonumber[]) {
  tft.fillScreen(darkgrey);
  tft.setTextSize(3);
  tft.setTextColor(white);
  tft.setCursor(getTextCenter("Calling to", 120, 3), getTextCenter(30, 3));
  tft.print("Calling to");
  tft.setCursor(20, 70);
  for (i = 0; i < 11; i++) {
    tft.print(tonumber[i]);
  }
  tft.setCursor(20, 100);
  for (i = 11; i < 22; i++) {
    tft.print(tonumber[i]);
  }
  drawButton(20, 200, 200, 100, "END CALL", 3, maroon, red, false, true);

  fona.callPhone(tonumber);

  while (1) {
    if (ts.touched()) {
      tY = getY();
      if (tY >= 200) {
        drawButton(20, 200, 200, 100, "END CALL", 3, maroon, red, true, false);
        while (ts.touched());
        drawButton(20, 200, 200, 100, "END CALL", 3, maroon, red, false, false);
        fona.hangUp();
        break;
      }
    }
  }

  for (i = 0; i < 30; i++) {
    phoneNumber[i] = ' ';
    phoneNumberIndex = 0;
  }
  draw(SCREEN_PHONE, false);
}

void callFrom() {
  bool ans = 0;
  tft.fillScreen(darkgrey);
  tft.setTextSize(2);
  tft.setTextColor(white);
  tft.setCursor(getTextCenter("Incoming call...", 120, 2), getTextCenter(40, 2));
  tft.print("Incoming call...");
  /*tft.setTextSize(3);
    fona.incomingCallNumber(incNumber);
    tft.setCursor(20, 70);
    tft.print("+");
    for (i = 0; i < 10; i++) {
    tft.print(incNumber[i]);
    }
    tft.setCursor(20, 100);
    for (i = 10; i < 21; i++) {
    tft.print(incNumber[i]);
    }*/
  drawButton(20, 140, 200, 70, "ANSWER", 3, darkgreen, green, false, true);
  drawButton(20, 230, 200, 70, "END CALL", 3, maroon, red, false, true);

  while (1) {
    if (!fona.getCallStatus()) {
      break;
    }
    if (ts.touched()) {
      tY = getY();
      if (tY >= 140 && tY <= 210 && !ans) {
        ans = 1;
        drawButton(20, 140, 200, 70, "ANSWER", 3, darkgreen, green, true, false);
        while (ts.touched());
        fona.pickUp();
        tft.fillRect(20, 140, 200, 70, darkgrey);
      }
      if (tY >= 230) {
        drawButton(20, 230, 200, 70, "END CALL", 3, maroon, red, true, false);
        while (ts.touched());
        drawButton(20, 230, 200, 70, "END CALL", 3, maroon, red, false, false);
        fona.hangUp();
      }
    }
  }
  draw(openApp, false);
}

void buzz(int dur) {
  fona.setPWM(2000);
  delay(dur);
  fona.setPWM(0);
}

void drawButton(int drawX, int drawY, int drawW, int drawH, char text[], int fontS, int dcolor, int lcolor, bool pressed, bool bgDraw) {
  if (bgDraw) {
    tft.fillRect(drawX, drawY, drawW, drawH, lcolor);
    drawTRect(drawX, drawY, drawW, drawH, dcolor, 3);
  }
  if (pressed) {
    buzz(5);
    tft.setTextColor(white, lcolor);
  } else {
    tft.setTextColor(dcolor, lcolor);
  }
  tft.setTextSize(fontS);
  tft.setCursor(getTextCenter(text, drawX + drawW / 2, fontS), getTextCenter(drawY + drawH / 2, fontS));
  tft.print(text);
}

void drawMenuButton(int drawX, int drawY, int drawW, int drawH, char text[], int fontS, int color, bool pressed, bool bgDraw) {
  if (bgDraw) {
    tft.fillRect(drawX, drawY, drawW, drawH, white);
    drawTRect(drawX, drawY, drawW, drawH, black, 3);
  }
  if (pressed) {
    buzz(5);
    tft.setTextColor(lightgrey, white);
  } else {
    tft.setTextColor(color, white);
  }
  tft.setTextSize(fontS);
  tft.setCursor(getTextCenter(text, drawX + drawW / 2, fontS), getTextCenter(drawY + drawH / 2, fontS));
  tft.print(text);
}

void appRoutine(byte app) {
  drawTime(app);
  drawBatt(app);
  switch (app) {
    case SCREEN_HOME:
      break;
    case SCREEN_MENU:
      break;
    case SCREEN_LOCK:
      break;
    case SCREEN_PHONE:
      break;
  }
}

void drawBatt(byte screen) {
  int toY = 187;
  char battLen = 10;
  switch (screen) {
    case SCREEN_HOME:
      tft.setTextSize(3);
      getBatt();
      if (batt < 100) {
        battLen = 9;
      }
      if (batt < 10) {
        battLen = 8;
      }
      tft.setCursor(getTextCenter(battLen, 120, 3), getTextCenter(85, 3));
      tft.setTextColor(white, navy);
      tft.print("Batt: ");
      tft.print(batt);
      tft.print('%');
      break;
    case SCREEN_MENU:
      break;
    case SCREEN_PHONE:
      tft.setTextSize(2);
      getBatt();
      if (batt < 100) {
        toY += 12;
      }
      if (batt < 10) {
        toY += 12;
      }
      tft.setCursor(toY, getTextCenter(10, 2));
      tft.setTextColor(white, red);
      tft.print(batt);
      tft.print('%');
      break;
    case SCREEN_SMS1:
    case SCREEN_SMS2:
      tft.setTextSize(2);
      getBatt();
      if (batt < 100) {
        toY += 12;
      }
      if (batt < 10) {
        toY += 12;
      }
      tft.setCursor(toY, getTextCenter(10, 2));
      tft.setTextColor(white, green);
      tft.print(batt);
      tft.print('%');
      break;
    case SCREEN_SET:
      tft.setTextSize(2);
      getBatt();
      if (batt < 100) {
        toY += 12;
      }
      if (batt < 10) {
        toY += 12;
      }
      tft.setCursor(toY, getTextCenter(10, 2));
      tft.setTextColor(white, lightgrey);
      tft.print(batt);
      tft.print('%');
      break;
  }
}

void getBatt() {
  fona.getBattPercent(&batt);
}

void drawTime(byte screen) {
  switch (screen) {
    case SCREEN_HOME:
      tft.setTextSize(6);
      tft.setCursor(getTextCenter(time, 120, 6), getTextCenter(40, 6));
      tft.setTextColor(white, navy);
      getTime();
      tft.print(time);
      break;
    case SCREEN_MENU:
      break;
    case SCREEN_PHONE:
      tft.setTextSize(2);
      tft.setCursor(5, getTextCenter(10, 2));
      tft.setTextColor(white, red);
      getTime();
      tft.print(time);
      break;
    case SCREEN_SMS1:
    case SCREEN_SMS2:
      tft.setTextSize(2);
      tft.setCursor(5, getTextCenter(10, 2));
      tft.setTextColor(white, green);
      getTime();
      tft.print(time);
      break;
    case SCREEN_SET:
      tft.setTextSize(2);
      tft.setCursor(5, getTextCenter(10, 2));
      tft.setTextColor(white, lightgrey);
      getTime();
      tft.print(time);
      break;
  }
}

void getTime() {
  char buff[23];
  fona.getTime(buff, 32);
  for (i = 10; i < 15; i++) {
    time[i - 10] = buff[i];
  }
}

void draw(byte screen, bool doAnim) {
  switch (screen) {
    case SCREEN_HOME:
      tft.fillScreen(navy);
      drawButton(30, 190, 180, 100, "Open Menu", 3, darkgrey, lightgrey, false, true);
      break;
    case SCREEN_MENU:
      tft.fillRect(0, 0, 240, appHeight, navy);
      drawMenuButton(0, appHeight, 120, appHeight, appNames[0], 2, appColor[0], false, true);
      drawMenuButton(120, appHeight, 120, appHeight, appNames[1], 2, appColor[1], false, true);
      drawMenuButton(0, appHeight * 2, 120, appHeight, appNames[2], 2, appColor[2], false, true);
      drawMenuButton(120, appHeight * 2, 120, appHeight, appNames[3], 2, appColor[3], false, true);
      drawMenuButton(0, appHeight * 3, 120, appHeight, appNames[4], 2, appColor[4], false, true);
      drawMenuButton(120, appHeight * 3, 120, appHeight, appNames[5], 2, appColor[5], false, true);
      drawMenuButton(0, appHeight * 4, 120, appHeight, appNames[6], 2, appColor[6], false, true);
      drawMenuButton(120, appHeight * 4, 120, appHeight, appNames[7], 2, appColor[7], false, true);
      break;
    case SCREEN_LOCK:
      tft.fillScreen(white);
      drawTRect(10, 20, 220, 90, black, 3);
      drawNumpad(0, 130);
      drawButton(NUMPAD_W, 130, 100, 95, "OK", 4, darkgreen, green, false, true);
      drawButton(NUMPAD_W, 225, 100, 95, "RMV", 4, maroon, red, false, true);
      break;
    case SCREEN_PHONE:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, red);
      drawTRect(10, 30, 220, 90, black, 3);
      drawNumpad(0, 130);
      drawButton(NUMPAD_W, 130, 100, 63, "CALL", 3, darkgreen, green, false, true);
      drawButton(NUMPAD_W, 193, 100, 64, "RMV", 3, maroon, red, false, true);
      drawButton(NUMPAD_W, 257, 100, 63, "CLEAR", 3, darkgrey, lightgrey, false, true);
      tft.setCursor(20, 40);
      tft.setTextSize(3);
      tft.setTextColor(black, white);
      for (i = 0; i < 11; i++) {
        tft.print(phoneNumber[i]);
      }
      tft.setCursor(20, 70);
      for (i = 11; i < 22; i++) {
        tft.print(phoneNumber[i]);
      }
      break;
    case SCREEN_SMS1:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, green);
      break;
    case SCREEN_SMS2:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, green);
      break;
    case SCREEN_SET:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, lightgrey);
      tft.setTextSize(3);
      tft.setTextColor(black);
      a = 0;
      for (i = 90; i < 320; i += 70) {
        tft.drawFastHLine(0, i, 240, darkgrey);
        tft.setCursor(20, getTextCenter(i + 35, 3));
        tft.print(settings[a]);
        a++;
      }
      break;
  }
  openApp = screen;
  updateTimer = -10001;
}

void drawNumpad(byte x, byte y) {
  tft.fillRect(x, y, NUMPAD_W, NUMPAD_H, lightgrey);
  drawTRect(x, y, NUMPAD_W, NUMPAD_H, black, 3);
  tft.setTextColor(black);
  tft.setTextSize(3);
  for (int i = 0; i < 12; i++) {
    tft.setCursor(x + kpX[i], y + kpY[i]);
    tft.print(kpC[i]);
  }
  numpadX = x;
  numpadY = y;
}

int getNumpad() {
  int row = 0;
  int col = 0;
  int numPressed = 0;
  if (tX >= numpadX && tX <= numpadX + NUMPAD_W / 3) {
    col = 1;
  } else if (tX > numpadX + NUMPAD_W / 3 && tX <= numpadX + NUMPAD_W / 3 * 2) {
    col = 2;
  } else if (tX > numpadX + NUMPAD_W / 3 * 2 && tX <= numpadX + NUMPAD_W) {
    col = 3;
  }
  if (tY >= numpadY && tY <= numpadY + NUMPAD_H / 4) {
    row = 1;
  } else if (tY > numpadY + NUMPAD_W / 4 && tY <= numpadY + NUMPAD_H / 4 * 2) {
    row = 2;
  } else if (tY > numpadY + NUMPAD_W / 4 * 2 && tY <= numpadY + NUMPAD_H / 4 * 3) {
    row = 3;
  } else if (tY > numpadY + NUMPAD_W / 4 * 3 && tY <= numpadY + NUMPAD_H) {
    row = 4;
  }
  Serial.print("Col: ");
  Serial.print(col);
  Serial.print(", row: ");
  Serial.println(row);
  if (col == 1) {
    if (row == 1) {
      numPressed = 0;
    } else if (row == 2) {
      numPressed = 3;
    } else if (row == 3) {
      numPressed = 6;
    } else if (row == 4) {
      numPressed = 9;
    }
  } else if (col == 2) {
    if (row == 1) {
      numPressed = 1;
    } else if (row == 2) {
      numPressed = 4;
    } else if (row == 3) {
      numPressed = 7;
    } else if (row == 4) {
      numPressed = 10;
    }
  } else if (col == 3) {
    if (row == 1) {
      numPressed = 2;
    } else if (row == 2) {
      numPressed = 5;
    } else if (row == 3) {
      numPressed = 8;
    } else if (row == 4) {
      numPressed = 11;
    }
  }
  Serial.print(numPressed);
  Serial.print(" = ");
  Serial.println(kpC[numPressed]);
  return numPressed;
}

int getX() {
  TP = ts.getPoint();
  TP.x = map(TP.x, 0, 240, 240, 0);
  return TP.x;
}

int getY() {
  TP = ts.getPoint();
  TP.y = map(TP.y, 0, 320, 320, 0);
  return TP.y;
}

int getColor(int r, int g, int b) {
  return ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
}

int getTextLength(char text[], int fontS) {
  int length = getChars(text) * (6 * fontS);
  return length;
}

int getChars(char text[]) {
  String tempText = String(text);
  return tempText.length();
}

int getTextCenter(char text[], int x, int font) {
  return x - getTextLength(text, font) / 2;
}

int getTextCenter(int length, int x, int font) {
  return x - (length * (font * 6)) / 2;
}

int getTextCenter(int y, int font) {
  return y - 3.5 * font;
}

void drawTRect(int x, int y, int w, int h, int color, int t) {
  for (i = 0; i < t; i++) {
    tft.drawRect(x + i, y + i, w - i * 2, h - i * 2, color);
  }
}

bool homeBtn() {
  return !digitalRead(18);
}




















#define BUFFPIXEL 40

int drawBMP(char *filename, uint8_t x, uint16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if ((x >= tft.width()) || (y >= tft.height())) return -1;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return -1;
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if ((x + w - 1) >= tft.width())  w = tft.width()  - x;
        if ((y + h - 1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

        for (row = 0; row < h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if (bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col = 0; col < w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.color565(r, g, b));
          } // end pixel
        } // end scanline
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if (!goodBmp) Serial.println(F("BMP format not recognized."));
  return bmpHeight;
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
