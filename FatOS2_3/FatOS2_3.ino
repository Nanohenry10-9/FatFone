#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <Adafruit_FONA.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

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
#define FONA_KEY A0
#define FONA_PWR A1
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
#define SCREEN_SET2   7
#define SCREEN_CONT   8
#define SCREEN_RADIO  9
#define SCREEN_CALC   10
#define SCREEN_PONG   11
#define SCREEN_SMSS   12
#define SCREEN_UNO    13
#define SCREEN_CHIS   14

#define NUMPAD_W  140
#define NUMPAD_H  190

#define SMS_MSG 0
#define PHONE_CALL 1
#define OTHER 2

bool rot = 0;

byte bl = 100;
byte vol = 50;
byte aud = 1;

char* appNames[] = {"Phone", "SMS", "Settings", "Contacts", "Calc", "Radio", "Clock", "Pong"};
int appColor[] = {red, green, darkgrey, blue, orange, navy, red, black};
byte numOfApps = 8;
#define appHeight 64 // Screen height (320) / 5

int tX = 0;
int tY = 0;
int oldX;
int oldY;
int lastX;
int lastY;
TS_Point TP;

int i = 0;
int a = 0;

byte openApp = SCREEN_MENU;

char kpC[] = {
  '1', '2', '3',
  '4', '5', '6',
  '7', '8', '9',
  '+', '0', '#'
};

byte kpX[] = {
  18,  61,  104,
  18,  61,  104,
  18,  61,  104,
  18,  61,  104
};

byte kpY[] = {
  20,  20,  20,
  65,  65,  65,
  108, 108, 108,
  150, 150, 150
};

char charsL[40] = {"abcdefghijklmnopqrstuvwxyz_.!?,-:UPC"};
char charsU[40] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ_.!?,()UPC"};
bool upperCase = false;

char smsMsg[255] = {};
int smsI = 0;
int maxSMS = 17;

char time[] = {"00:00"};
uint16_t batt = 100;
uint16_t oldBatt = 0;
uint16_t newBatt = 0;
bool charging = false;

bool airplane = false;

long updateTimer = 0;
long slideTimer = 0;

char phoneNumber[24] = {' '};
int phoneNumberIndex = 0;
char chistory[5][24] = {"12345678998765432112345", "987654321", "123321123", "111222333", "147258369"};
bool called[5] = {true, false, false, true, true};
bool answered[5] = {true, false, true, false, true};

int numpadX = 0;
int numpadY = 0;
int keypadX = 0;
int keypadY = 0;
int lastKey = -1;
int keyCount = 0;

char psword[] = {"1234"};
char inputpw[] = {"    "};
int inputpwindex = 0;

char* settings[4] = {"General", "Battery", "Telephony", "Shutdown"};
byte selectedSet = -1;

struct Contact {
  char name[20];
  char number[22];
};

Contact per1 = {"Test1", "+555 12 34 56"};
Contact per2 = {"Test2", "+555 12 34 56"};
Contact per3 = {"Test3", "+555 12 34 56"};
Contact per4 = {"Test4", "+555 12 34 56"};

Contact contacts[4] = {per1, per2, per3, per4};

int newMsgs[4] = {false, false, false, false};
char msgs1[6][100] = {"", "", "", ""};
char msgs2[6][100] = {"", "", "", ""};
char msgs3[6][100] = {"", "", "", ""};
char msgs4[6][100] = {"", "", "", ""};
bool oMsg[4][6] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
int ms1[4] = {0, 0, 0, 0};
int smsChat = -1;

int oldSMSNum = 0;

char smsBuff[100] = {""};

char* calcButtons[] = {"+-*/s="};

byte cycles = 0;

void setup() {
  Serial.begin(115200);
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  pinMode(FONA_PWR, INPUT);
  pinMode(FONA_KEY, OUTPUT);
  digitalWrite(FONA_KEY, HIGH);
  tft.begin();
  if (!rot) {
    tft.setRotation(0);
  } else {
    tft.setRotation(2);
  }
  analogWrite(TFT_BL, 100);


  /*tft.fillScreen(navy); // 191 - 219: only for demo
    tft.setCursor(10, 10);
    tft.setTextSize(5);
    tft.setTextColor(red);
    tft.print("H");
    tft.setTextColor(orange);
    tft.print("o");
    tft.setTextColor(yellow);
    tft.print("w");
    tft.setTextColor(blue);
    tft.print("d");
    tft.setTextColor(green);
    tft.print("y");
    tft.setTextColor(cyan);
    tft.print("!");
    tft.setCursor(10, 70);
    tft.setTextColor(white);
    tft.setTextSize(2);
    tft.print("Yes, this is a");
    tft.setCursor(10, 90);
    tft.print("mobile phone.");
    tft.setCursor(10, 130);
    tft.print("Touch the screen!");
    tft.setCursor(10, 170);
    tft.print("By the way, the");
    tft.setCursor(10, 190);
    tft.print("passcode is XXXX");
    ts.begin();
    while (!ts.touched());*/


  tft.fillScreen(navy);
  tft.fillRect(40, 120, 160, 70, lightgrey);
  drawTRect(40, 120, 160, 70, darkgrey, 3);
  tft.setTextColor(black, lightgrey);
  tft.setTextSize(2);
  tft.setCursor(getTextXCenter(7, 120, 2), getTextYCenter(155, 2));
  tft.print("Loading");

  if (!digitalRead(FONA_PWR)) {
    digitalWrite(FONA_KEY, LOW);
    delay(2100); // Delay to get FONA started
    digitalWrite(FONA_KEY, HIGH);
    while (!digitalRead(FONA_PWR));
  }

  fonaSerial->begin(4800);

  fona.begin(*fonaSerial);

  fona.callerIdNotification(true, digitalPinToInterrupt(3));
  fona.setPWM(0);
  if (!EEPROM.read(0)) {
    EEPROM.write(0, 1);
    EEPROM.write(1, 50);
    EEPROM.write(2, 100);
    EEPROM.write(3, 1);
  }
  vol = EEPROM.read(1);
  bl = EEPROM.read(2);
  aud = EEPROM.read(3);
  fona.setAllVolumes(vol);
  if (bl >= 100) {
    for (i = 100; i <= bl; i++) {
      analogWrite(TFT_BL, i);
      delay(3);
    }
  } else {
    for (i = 100; i >= bl; i--) {
      analogWrite(TFT_BL, i);
      delay(3);
    }
  }
  fona.setAudio((bool)aud);

  tft.setCursor(getTextXCenter(12, 120, 2), getTextYCenter(155, 2));
  tft.print("   Ready!   ");
  delay(1000);
  draw(SCREEN_LOCK, 0);
}

void loop() {
  if (ts.touched()) {
    Serial.print("Touched, app: ");
    Serial.println(openApp);
    touchHandler(openApp);
  }

  if (millis() - updateTimer >= 10000) {
    Serial.println("Timeout, updating data...");
    appRoutine(openApp);
    updateTimer = millis();
    /*cycles++;
      if (cycles >= 6) {
      asm volatile ("jmp 0");
      }*/
  }

  if (homeBtn() && openApp != SCREEN_MENU && openApp != SCREEN_LOCK) {
    draw(SCREEN_MENU, true);
  }

  if (homeBtn() && openApp == SCREEN_MENU) {
    draw(SCREEN_HOME, true);
  }

  if (newSMS() && openApp != SCREEN_LOCK) {
    char sender[22];
    int num = fona.getNumSMS();
    fona.readSMS(num, smsBuff, 100, NULL);
    fona.getSMSSender(num, sender, 22);
    byte newSMSchat = 0;
    bool equal = true;
    bool found = false;
    for (a = 0; a < 4; a++) {
      equal = true;
      for (i = 0; i < getChars(sender); i++) {
        Serial.print(sender[i]);
        Serial.print(F(" = "));
        Serial.print(contacts[a].number[i]);
        Serial.println(F("?"));
        if (isDigit(sender[i])) {
          if (sender[i] != contacts[a].number[i]) {
            equal = false;
          }
        }
      }
      Serial.println(F("Next contact..."));
      if (equal) {
        newSMSchat = a;
      }
    }
    addSMS(newSMSchat, smsBuff, false);
    if (openApp != SCREEN_SMS2) {
      popup("New SMS received", "View", SMS_MSG);
    } else {
      tft.fillRect(0, 20, 240, 240, white);
      tft.setTextSize(2);
      tft.setTextColor(black);
      for (a = 0; a < ms1[smsChat]; a++) {
        tft.setCursor(10, (a + 1) * 40);
        if (oMsg[smsChat][a]) {
          tft.print("You: ");
          Serial.print("You: ");
        } else {
          tft.print(contacts[smsChat].name);
          Serial.print(contacts[smsChat].name);
          tft.print(": ");
          Serial.print(": ");
        }
        if (smsChat == 0) {
          tft.print(msgs1[a]);
          Serial.println(msgs1[a]);
        } else if (smsChat == 1) {
          tft.print(msgs2[a]);
          Serial.println(msgs2[a]);
        } else if (smsChat == 2) {
          tft.print(msgs3[a]);
          Serial.println(msgs3[a]);
        } else if (smsChat == 3) {
          tft.print(msgs4[a]);
          Serial.println(msgs4[a]);
        } else {
          tft.print(F("Msg not found"));
          Serial.println(F("Msg not found"));
        }
      }
    }
  }

  if (incCall() && openApp != SCREEN_LOCK) {
    bool answered = callFrom();
    if (!answered && openApp != SCREEN_CHIS) {
      popup("Missed call", "View", PHONE_CALL);
    }
  }

  if (homeBtn()) {
    switch (openApp) {
      case SCREEN_SMS2:
        draw(SCREEN_SMS1, false);
        break;
      case SCREEN_SMSS:
        draw(SCREEN_SMS2, false);
        break;
      case SCREEN_SET2:
        draw(SCREEN_SET, false);
        break;
    }
  }
}

void touchHandler(byte screen) {
  tX = getX();
  tY = getY();
  /*Serial.print("TS touched [X, Y]: ");
    Serial.print(tX);
    Serial.print(", ");
    Serial.println(tY);*/
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
          draw(SCREEN_CALC, 1);
        } else if (tY > appHeight * 4) {
          drawTRect(0, appHeight * 4, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_UNO, 1);
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
          draw(SCREEN_PONG, 1);
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
          tft.setCursor(getTextXCenter(inputpwindex, 120, 4), getTextYCenter(65, 4));
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
        } else if (inputpw[0] == '4' && inputpw[1] == '2' && inputpw[2] == '0' && inputpw[3] == ' ') {
          while (ts.touched());
          drawButton(NUMPAD_W, 130, 100, 95, "OK", 4, darkgreen, green, false, false);
          for (int a = 0; a < 50; a++) {
            tft.setCursor(getTextXCenter(inputpwindex, 120, 4), getTextYCenter(65, 4));
            tft.setTextColor(random(0, 65536), white);
            tft.setTextSize(4);
            tft.print(inputpw);
          }
          tft.setCursor(getTextXCenter(inputpwindex, 120, 4), getTextYCenter(65, 4));
          tft.setTextColor(black, white);
          tft.setTextSize(4);
          tft.print(inputpw);
        } else if (inputpw != "    ") {
          tft.setCursor(getTextXCenter(inputpwindex, 120, 4), getTextYCenter(65, 4));
          tft.setTextColor(red, white);
          tft.setTextSize(4);
          tft.print(inputpw);
          delay(1000);
          tft.setTextColor(black, white);
          for (i = 0; i < 4; i++) {
            inputpw[i] = ' ';
          }
          inputpwindex = 0;
          tft.setCursor(getTextXCenter(4, 120, 4), getTextYCenter(65, 4));
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
          tft.setCursor(getTextXCenter(inputpwindex, 120, 4) - 24, getTextYCenter(65, 4));
          tft.setTextColor(black, white);
          tft.setTextSize(4);
          tft.print(" ");
          tft.print(inputpw);
        }
        while (ts.touched());
        drawButton(NUMPAD_W, 225, 100, 95, "RMV", 4, maroon, red, false, false);
      }
      break;
    case SCREEN_PHONE:
      oldX = getX();
      tX = oldX;
      lastX = oldX;
      slideTimer = millis();
      while (true) {
        if (ts.touched()) {
          tX = getX();
          if (tX == 240) {
            tX = lastX;
          } else {
            lastX = tX;
          }
        } else {
          break;
        }
      }
      if (tX - oldX <= -10) { // If swiped
        if (millis() - slideTimer <= 500) {
          screen = SCREEN_CHIS;
          for (i = 240; i >= 0; i--) {
            tft.drawFastVLine(i, 20, 300, white);
          }
          draw(SCREEN_CHIS, false);
        }
      } else if (tX - oldX >= 10) {
        if (millis() - slideTimer <= 500) {
          screen = SCREEN_CHIS;
          for (i = 0; i < 240; i++) {
            tft.drawFastVLine(i, 20, 300, white);
          }
          draw(SCREEN_CHIS, false);
        }
      } else {
        tX = oldX;
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
          for (i = 21; i < 22; i++) {
            tft.print(phoneNumber[i]);
          }
          while (ts.touched());
          draw(SCREEN_PHONE, true);
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
      }
      break;
    case SCREEN_CHIS:
      oldX = getX();
      tX = oldX;
      lastX = oldX;
      slideTimer = millis();
      while (true) {
        if (ts.touched()) {
          tX = getX();
          if (tX == 240) {
            tX = lastX;
          } else {
            lastX = tX;
          }
        } else {
          break;
        }
      }
      if (tX - oldX <= -10) { // If swiped
        if (millis() - slideTimer <= 500) {
          screen = SCREEN_PHONE;
          for (i = 240; i >= 0; i--) {
            tft.drawFastVLine(i, 20, 300, white);
          }
          draw(SCREEN_PHONE, false);
        }
      } else if (tX - oldX >= 10) {
        if (millis() - slideTimer <= 500) {
          screen = SCREEN_PHONE;
          for (i = 0; i < 240; i++) {
            tft.drawFastVLine(i, 20, 300, white);
          }
          draw(SCREEN_PHONE, false);
        }
      } else {
        char numberBuffer[30];
        numberBuffer[0] = '+';
        if (tY >= 20 && tY < 90) {
          for (i = 0; i < 22; i++) {
            numberBuffer[i + 1] = chistory[0][i];
          }
          callTo(numberBuffer);
        }
        if (tY >= 90 && tY < 150) {
          for (i = 0; i < 22; i++) {
            numberBuffer[i + 1] = chistory[1][i];
          }
          callTo(numberBuffer);
        }
        if (tY >= 150 && tY < 210) {
          for (i = 0; i < 22; i++) {
            numberBuffer[i + 1] = chistory[2][i];
          }
          callTo(numberBuffer);
        }
        if (tY >= 210 && tY < 270) {
          for (i = 0; i < 22; i++) {
            numberBuffer[i + 1] = chistory[3][i];
          }
          callTo(numberBuffer);
        }
        if (tY >= 270) {
          for (i = 0; i < 22; i++) {
            numberBuffer[i + 1] = chistory[4][i];
          }
          callTo(numberBuffer);
        }
      }
      break;
    case SCREEN_SMS1:
      if (tY >= 20 && tY <= 90) {
        smsChat = 0;
        draw(SCREEN_SMS2, false);
      }
      if (tY > 90 && tY <= 160) {
        smsChat = 1;
        draw(SCREEN_SMS2, false);
      }
      if (tY > 160 && tY <= 230) {
        smsChat = 2;
        draw(SCREEN_SMS2, false);
      }
      if (tY > 230 && tY <= 300) {
        smsChat = 3;
        draw(SCREEN_SMS2, false);
      }
      break;
    case SCREEN_SMS2:
      if (tY >= 160) {
        drawButton(0, 260, 240, 60, "SEND MESSAGE", 3, darkgreen, green, true, false);
        while (ts.touched());
        draw(SCREEN_SMSS, false);
      }
      break;
    case SCREEN_SMSS:
      if (tX <= NUMPAD_W && tX >= 0 && tY >= 130 && tY <= 319) {
        int keyPressed = getNumpad();
        tft.setTextSize(2);

        if (keyPressed == 11) {
          if (upperCase) {
            tft.setTextColor(black, lightgrey);
            upperCase = false;
          } else {
            tft.setTextColor(white, lightgrey);
            upperCase = true;
          }
          tft.setCursor(getTextXCenter(3, kpX[11], 1), 130 + kpY[11]);
          for (i = 33; i < 36; i++) {
            tft.print(charsL[i]);
          }
          buzz(5);

          tft.setTextColor(black, lightgrey);
          int letrs = 0;
          for (i = 0; i < 11; i++) {
            tft.setCursor(getTextXCenter(3, kpX[i], 1), 130 + kpY[i]);
            for (a = 0; a < 3; a++) {
              if (upperCase) {
                tft.print(charsU[letrs]);
                Serial.print(charsU[letrs]);
              } else {
                tft.print(charsL[letrs]);
                Serial.print(charsL[letrs]);
              }
              letrs++;
            }
            Serial.println();
          }
        } else {
          while (true) {
            if (ts.touched()) {
              tX = getX();
            } else {
              break;
            }
          }
          if (tX <= 80) {
            keyPressed = keyPressed * 3;
          } else if (tX > 80 && tX < 160) {
            keyPressed = keyPressed * 3 + 1;
          } else if (tX >= 160) {
            keyPressed = keyPressed * 3 + 2;
          }

          tX = 0;
          tY = 0;

          char charPressed = '\0';
          if (upperCase) {
            charPressed = charsU[keyPressed];
          } else {
            charPressed = charsL[keyPressed];
          }

          if (charPressed == '_') {
            charPressed = ' ';
          }

          if (smsI < maxSMS) {
            smsMsg[smsI] = charPressed;
            smsI++;
            tft.setTextColor(black, white);
            tft.setTextSize(2);
            tft.setCursor(20, 40);
            for (i = 0; i < maxSMS; i++) {
              tft.print(smsMsg[i]);
            }
          }

          buzz(5);
          while (ts.touched());

        }
      }
      if (tX >= NUMPAD_W && tX <= NUMPAD_W + 100 && tY >= 130 && tY <= 193) {
        drawButton(NUMPAD_W, 130, 100, 63, "SEND", 3, darkgreen, green, true, false);
        while (ts.touched());
        // Send SMS (FONA)
        smsMsg[smsI] = '\0';
        fona.sendSMS(contacts[smsChat].number, smsMsg);
        addSMS(smsChat, smsMsg, true);

        for (i = 0; i < maxSMS; i++) {
          smsMsg[i] = ' ';
        }
        smsI = 0;
        draw(SCREEN_SMS2, false);
      }

      if (tX >= NUMPAD_W && tX <= NUMPAD_W + 100 && tY > 193 && tY <= 257) {
        drawButton(NUMPAD_W, 193, 100, 64, "RMV", 3, maroon, red, true, false);
        if (smsI > 0) {
          smsI--;
          smsMsg[smsI] = ' ';
          tft.setTextColor(black, white);
          tft.setTextSize(2);
          tft.setCursor(20, 40);
          for (i = 0; i < maxSMS; i++) {
            tft.print(smsMsg[i]);
          }
        }
        while (ts.touched());
        drawButton(NUMPAD_W, 193, 100, 64, "RMV", 3, maroon, red, false, false);
      }

      if (tX >= NUMPAD_W && tX <= NUMPAD_W + 100 && tY > 257 && tY < 319) {
        drawButton(NUMPAD_W, 257, 100, 63, "CLEAR", 3, darkgrey, lightgrey, true, false);
        for (i = 0; i < maxSMS; i++) {
          smsMsg[i] = ' ';
        }
        smsI = 0;
        tft.setTextColor(black, white);
        tft.setTextSize(2);
        tft.setCursor(20, 40);
        for (i = 0; i < maxSMS; i++) {
          tft.print(smsMsg[i]);
        }
        while (ts.touched());
        drawButton(NUMPAD_W, 257, 100, 63, "CLEAR", 3, darkgrey, lightgrey, false, false);
      }
      break;
    case SCREEN_SET:
      if (tY >= 20 && tY <= 90) {
        selectedSet = 0;
        draw(SCREEN_SET2, false);
      }
      if (tY > 90 && tY <= 160) {
        selectedSet = 1;
        draw(SCREEN_SET2, false);
      }
      if (tY > 160 && tY <= 230) {
        selectedSet = 2;
        draw(SCREEN_SET2, false);
      }
      if (tY > 230 && tY <= 300) {
        shutdown();
      }
      break;
    case SCREEN_SET2:
      switch (selectedSet) {
        case 0:
          if (tY > 60 && tY < 100) {
            while (ts.touched() && tX > 20 && tX < 210) {
              tft.fillRect(map(bl, 0, 255, 20, 210), 60, 10, 40, white);
              tft.drawFastHLine(20, 80, 200, black);
              tft.drawFastVLine(20, 60, 40, black);
              tft.drawFastVLine(220, 60, 40, black);
              tft.drawFastVLine(120, 65, 30, black);
              tft.drawFastVLine(70, 70, 20, black);
              tft.drawFastVLine(170, 70, 20, black);
              bl = map(tX, 20, 210, 0, 255);
              tft.fillRect(map(bl, 0, 255, 20, 210), 60, 10, 40, darkgrey);
              analogWrite(TFT_BL, bl);
              tX = getX();
            }
          }
          if (tY > 140 && tY < 180) {
            while (ts.touched() && tX > 20 && tX < 210) {
              tft.fillRect(map(vol, 0, 100, 20, 210), 140, 10, 40, white);
              tft.drawFastHLine(20, 160, 200, black);
              tft.drawFastVLine(20, 140, 40, black);
              tft.drawFastVLine(220, 140, 40, black);
              tft.drawFastVLine(120, 145, 30, black);
              tft.drawFastVLine(70, 150, 20, black);
              tft.drawFastVLine(170, 150, 20, black);
              vol = map(tX, 20, 210, 0, 100);
              tft.fillRect(map(vol, 0, 100, 20, 210), 140, 10, 40, darkgrey);
              tX = getX();
            }
            fona.setAllVolumes(vol);
            fona.playToolkitTone(6, 1000);
          }
          break;
        case 1:

          break;
        case 2:

          break;
      }
      break;
  }
}

void popup(char msg[], char btn1[], byte type) {
  tft.fillRect(10, 20, 220, 150, lightgrey);
  drawTRect(10, 20, 220, 150, black, 3);
  drawTRect(10, 20, 220, 100, black, 3);
  tft.setTextSize(2);
  tft.setTextColor(black);
  tft.setCursor(getTextXCenter(getChars(msg), 120, 2), getTextYCenter(70, 2));
  tft.print(msg);
  tft.setTextSize(3);
  tft.setCursor(getTextXCenter(getChars(btn1), 120, 3), getTextYCenter(145, 3));
  tft.print(btn1);
  while (!ts.touched());
  switch (type) {
    case SMS_MSG:
      smsChat = 0;
      draw(SCREEN_SMS2, false);
      break;
    case PHONE_CALL:
      draw(SCREEN_CHIS, true);
      break;
    case OTHER:
      draw(openApp, true);
      break;
  }
}

void callTo(char tonumber[]) {
  fona.callPhone(tonumber);

  tft.fillScreen(darkgrey);
  tft.setTextSize(3);
  tft.setTextColor(white);
  tft.setCursor(getTextXCenter(10, 120, 3), getTextYCenter(30, 3));
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

  while (1) {
    Serial.println(fona.getCallStatus());
    if (!fona.getCallStatus()) {
      Serial.println("Call ended");
      break;
    }
    if (ts.touched()) {
      tY = getY();
      if (tY >= 200) {
        drawButton(20, 200, 200, 100, "END CALL", 3, maroon, red, true, false);
        while (ts.touched());
        drawButton(20, 200, 200, 100, "END CALL", 3, maroon, red, false, false);
        fona.hangUp();
      }
    }
  }
  fona.hangUp();
  tX = 0;
  tY = 0;
  for (i = 4; i > 0; i--) {
    for (a = 0; a < 22; a++) {
      chistory[i][a] = chistory[i - 1][a];
    }
    answered[i] = answered[i - 1];
    called[i] = called[i - 1];
  }
  for (i = 0; i < 22; i++) {
    chistory[0][i] = tonumber[i + 1];
  }
  answered[0] = 1;

  for (i = 0; i < 30; i++) {
    phoneNumber[i] = ' ';
    phoneNumberIndex = 0;
  }
  draw(openApp, true);
}

bool callFrom() {
  bool ans = 0;
  char incNumber[22] = {' '};
  tft.fillScreen(darkgrey);
  tft.setTextSize(2);
  tft.setTextColor(white);
  tft.setCursor(getTextXCenter(13, 120, 2), getTextYCenter(40, 2));
  tft.print("Incoming call");

  drawButton(20, 140, 200, 70, "ANSWER", 3, darkgreen, green, false, true);
  drawButton(20, 230, 200, 70, "END CALL", 3, maroon, red, false, true);

  fona.setPWM(2000);

  tft.setTextSize(2);
  tft.setCursor(20, 70);
  tft.setTextColor(white, darkgrey);
  tft.print(F("Loading number"));
  fona.incomingCallNumber(incNumber);
  tft.setCursor(20, 70);
  tft.setTextSize(3);
  tft.print("+");
  for (i = 0; i < 10; i++) {
    if (isDigit(incNumber[i])) {
      tft.print(incNumber[i]);
    }
  }
  tft.setCursor(20, 100);
  for (i = 10; i < 22; i++) {
    if (isDigit(incNumber[i])) {
      tft.print(incNumber[i]);
    }
  }

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
        fona.setPWM(0);
        fona.pickUp();
        tft.fillRect(20, 140, 200, 70, darkgrey);
      }
      if (tY >= 230) {
        ans = 1;
        drawButton(20, 230, 200, 70, "END CALL", 3, maroon, red, true, false);
        while (ts.touched());
        drawButton(20, 230, 200, 70, "END CALL", 3, maroon, red, false, false);
        fona.hangUp();
      }
    }
  }
  fona.setPWM(0);
  for (i = 4; i > 0; i--) {
    for (a = 0; a < 22; a++) {
      chistory[i][a] = chistory[i - 1][a];
    }
    answered[i] = answered[i - 1];
    called[i] = called[i - 1];
  }
  for (i = 0; i < 22; i++) {
    if (isDigit(incNumber[i])) {
      chistory[0][i] = incNumber[i];
    }
  }
  answered[0] = ans;
  fona.hangUp();
  draw(openApp, true);
  tX = 0;
  tY = 0;
  return ans;
}

void buzz(int dur) {
  fona.setPWM(2000);
  delay(dur);
  fona.setPWM(0);
}

void addSMS(byte chat, char msg[100], bool own) {
  if (ms1[chat] == 5) {
    Serial.println(F("Shifting all SMS's..."));
    for (i = 0; i < 4; i++) {
      for (a = 0; a < 100; a++) {
        switch (chat) {
          case 0:
            msgs1[i][a] = msgs1[i + 1][a];
            break;
          case 1:
            msgs2[i][a] = msgs2[i + 1][a];
            break;
          case 2:
            msgs3[i][a] = msgs3[i + 1][a];
            break;
          case 3:
            msgs4[i][a] = msgs4[i + 1][a];
            break;
        }
      }
      oMsg[chat][i] = oMsg[chat][i + 1];
    }
    Serial.println(F("Adding new SMS to array..."));
    for (i = 0; i < 100; i++) {
      switch (chat) {
        case 0:
          msgs1[ms1[chat] - 1][i] = msg[i];
          break;
        case 1:
          msgs2[ms1[chat] - 1][i] = msg[i];
          break;
        case 2:
          msgs3[ms1[chat] - 1][i] = msg[i];
          break;
        case 3:
          msgs4[ms1[chat] - 1][i] = msg[i];
          break;
      }

    }
  } else {
    Serial.println(F("Adding new SMS to array..."));
    for (i = 0; i < 100; i++) {
      switch (chat) {
        case 0:
          msgs1[ms1[chat]][i] = msg[i];
          break;
        case 1:
          msgs2[ms1[chat]][i] = msg[i];
          break;
        case 2:
          msgs3[ms1[chat]][i] = msg[i];
          break;
        case 3:
          msgs4[ms1[chat]][i] = msg[i];
          break;
      }
    }
  }
  oMsg[chat][ms1[chat]] = own;
  if (ms1[chat] < 5) {
    ms1[chat]++;
  }
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
  tft.setCursor(getTextXCenter(getChars(text), drawX + drawW / 2, fontS), getTextYCenter(drawY + drawH / 2, fontS));
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
  tft.setCursor(getTextXCenter(getChars(text), drawX + drawW / 2, fontS), getTextYCenter(drawY + drawH / 2, fontS));
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
    case SCREEN_SET2:
      if (selectedSet == 1) {
        tft.setCursor(10, 30);
        tft.setTextColor(black, white);
        tft.setTextSize(2);
        tft.print(F("Battery: "));
        getBatt();
        tft.print(batt);
        tft.print('%');
        tft.setCursor(10, 60);
        tft.print("Charging: ");
        if (charging) {
          tft.print("Yes");
        } else {
          tft.print("No ");
        }
      }
      break;
  }
}

void drawBatt(byte screen) {
  switch (screen) {
    case SCREEN_HOME:
      tft.setTextSize(3);
      getBatt();
      int battLen;
      battLen = 10;
      if (batt < 100) {
        battLen = 9;
      } else if (batt < 10) {
        battLen = 8;
      }
      tft.setCursor(getTextXCenter(battLen, 120, 3), getTextYCenter(85, 3));
      if (charging) {
        tft.setTextColor(darkgreen, navy);
      } else {
        tft.setTextColor(white, navy);
      }
      tft.print("Batt: ");
      tft.print(batt);
      tft.print('%');
      break;
    case SCREEN_MENU:
      break;
    case SCREEN_PHONE:
      tft.setTextSize(2);
      getBatt();
      tft.setTextColor(white, red);
      tft.setCursor(4, getTextYCenter(10, 2));
      if (charging) {
        tft.setTextColor(darkgreen, red);
      } else {
        tft.setTextColor(white, red);
      }
      tft.print(batt);
      tft.print('%');
      break;
    case SCREEN_SMS1:
    case SCREEN_SMS2:
    case SCREEN_SMSS:
      tft.setTextSize(2);
      getBatt();
      tft.setTextColor(white, green);
      tft.setCursor(4, getTextYCenter(10, 2));
      if (charging) {
        tft.setTextColor(darkgreen, green);
      } else {
        tft.setTextColor(white, green);
      }
      tft.print(batt);
      tft.print('%');
      break;
    case SCREEN_SET:
    case SCREEN_SET2:
      tft.setTextSize(2);
      getBatt();
      tft.setTextColor(white, lightgrey);
      tft.setCursor(4, getTextYCenter(10, 2));
      if (charging) {
        tft.setTextColor(darkgreen, lightgrey);
      } else {
        tft.setTextColor(white, lightgrey);
      }
      tft.print(batt);
      tft.print('%');
      break;
  }
}

void getBatt() {
  fona.getBattPercent(&batt);
  uint16_t battNew;
  fona.getBattVoltage(&battNew);
  if (battNew != oldBatt) {
    if (oldBatt > batt) {
      charging = false;
    } else if (oldBatt < batt) {
      charging = true;
    }
    oldBatt = batt;
  }
}

void drawTime(byte screen) {
  getTime();
  switch (screen) {
    case SCREEN_HOME:
      tft.setTextSize(6);
      tft.setCursor(getTextXCenter(getChars(time), 120, 6), getTextYCenter(40, 6));
      tft.setTextColor(white, navy);
      tft.print(time);
      break;
    case SCREEN_MENU:
      tft.setTextSize(4);
      tft.setCursor(getTextXCenter(getChars(time), 120, 4), getTextYCenter(32, 4));
      tft.setTextColor(white, navy);
      tft.print(time);
      break;
    case SCREEN_PHONE:
      tft.setTextSize(2);
      tft.setCursor(getTextXCenter(getChars(time), 120, 2), getTextYCenter(10, 2));
      tft.setTextColor(white, red);
      tft.print(time);
      break;
    case SCREEN_SMS1:
    case SCREEN_SMS2:
    case SCREEN_SMSS:
      tft.setTextSize(2);
      tft.setCursor(getTextXCenter(getChars(time), 120, 2), getTextYCenter(10, 2));
      tft.setTextColor(white, green);
      tft.print(time);
      break;
    case SCREEN_SET:
      tft.setTextSize(2);
      tft.setCursor(getTextXCenter(getChars(time), 120, 2), getTextYCenter(10, 2));
      tft.setTextColor(white, lightgrey);
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
      tft.setCursor(getTextXCenter(getChars(inputpw), 120, 4), getTextYCenter(65, 4));
      tft.setTextColor(black, white);
      tft.setTextSize(4);
      tft.print(inputpw);
      break;
    case SCREEN_PHONE:
      if (doAnim) {
        tft.fillScreen(white);
        tft.fillRect(0, 0, 240, 20, red);
      }
      drawTRect(10, 30, 220, 70, black, 3);
      drawNumpad(0, 130);
      drawButton(NUMPAD_W, 130, 100, 63, "CALL", 3, darkgreen, green, false, true);
      drawButton(NUMPAD_W, 193, 100, 64, "RMV", 3, maroon, red, false, true);
      drawButton(NUMPAD_W, 257, 100, 63, "CLEAR", 3, darkgrey, lightgrey, false, true);
      tft.setTextColor(black, white);
      tft.setTextSize(1);
      tft.setCursor(getTextXCenter(36, 120, 1), getTextYCenter(115, 1));
      tft.print(F("Swipe left or right for call history"));
      tft.setCursor(20, 40);
      tft.setTextSize(3);
      for (i = 0; i < 11; i++) {
        tft.print(phoneNumber[i]);
      }
      tft.setCursor(20, 70);
      for (i = 11; i < 22; i++) {
        tft.print(phoneNumber[i]);
      }
      break;
    case SCREEN_CHIS:
      if (doAnim) {
        tft.fillScreen(white);
        tft.fillRect(0, 0, 240, 20, red);
      }
      tft.setTextSize(3);
      for (i = 0; i < 5; i++) {
        if (answered[i]) {
          tft.setTextColor(black, white);
        } else {
          tft.setTextColor(red, white);
        }
        tft.setCursor(21, (i + 1) * 60 - 30);
        tft.print(F("+"));
        for (a = 0; a < 11; a++) {
          tft.print(chistory[i][a]);
        }
        tft.setCursor(21, (i + 1) * 60 - 5);
        for (a = 11; a < 22; a++) {
          tft.print(chistory[i][a]);
        }
      }
      break;
    case SCREEN_SMS1:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, green);
      a = 0;
      tft.setTextColor(black, white);
      for (i = 90; i < 320; i += 70) {
        tft.drawFastHLine(0, i, 240, darkgrey);
        tft.setTextSize(2);
        tft.setCursor(20, getTextYCenter(i - 50, 2));
        if (newMsgs[a]) {
          tft.setTextColor(blue, white);
        }
        tft.print(contacts[a].name);
        tft.setCursor(20, getTextYCenter(i - 20, 2));
        tft.setTextColor(black, white);
        tft.print(contacts[a].number);
        a++;
      }
      break;
    case SCREEN_SMS2:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, green);
      tft.setTextSize(1);
      tft.setTextColor(black);
      for (a = 0; a < ms1[smsChat]; a++) {
        tft.setCursor(10, (a + 1) * 40);
        if (oMsg[smsChat][a]) {
          tft.print("You: ");
          Serial.print("You: ");
        } else {
          tft.print(contacts[smsChat].name);
          Serial.print(contacts[smsChat].name);
          tft.print(": ");
          Serial.print(": ");
        }
        if (smsChat == 0) {
          tft.print(msgs1[a]);
          Serial.println(msgs1[a]);
        } else if (smsChat == 1) {
          tft.print(msgs2[a]);
          Serial.println(msgs2[a]);
        } else if (smsChat == 2) {
          tft.print(msgs3[a]);
          Serial.println(msgs3[a]);
        } else if (smsChat == 3) {
          tft.print(msgs4[a]);
          Serial.println(msgs4[a]);
        } else {
          tft.print(F("Msg not found"));
          Serial.println(F("Msg not found"));
        }
      }
      drawButton(0, 260, 240, 60, "SEND MESSAGE", 3, darkgreen, green, false, true);
      break;
    case SCREEN_SMSS:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, green);
      drawTRect(10, 30, 220, 90, black, 3);
      drawKeypad(0, 130);
      drawButton(NUMPAD_W, 130, 100, 63, "SEND", 3, darkgreen, green, false, true);
      drawButton(NUMPAD_W, 193, 100, 64, "RMV", 3, maroon, red, false, true);
      drawButton(NUMPAD_W, 257, 100, 63, "CLEAR", 3, darkgrey, lightgrey, false, true);
      break;
    case SCREEN_SET:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, lightgrey);
      tft.setTextSize(3);
      tft.setTextColor(black);
      a = 0;
      for (i = 90; i < 320; i += 70) {
        tft.drawFastHLine(0, i, 240, darkgrey);
        tft.setCursor(20, getTextYCenter(i - 35, 3));
        if (a == 3) {
          tft.setTextColor(red);
        }
        tft.print(settings[a]);
        a++;
      }
      break;
    case SCREEN_SET2:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, lightgrey);
      tft.setTextSize(2);
      tft.setTextColor(black);
      switch (selectedSet) {
        case 0:
          tft.setCursor(getTextXCenter(9, 120, 2), 40);
          tft.print(F("Backlight"));
          tft.drawFastHLine(20, 80, 200, black);
          tft.drawFastVLine(20, 60, 40, black);
          tft.drawFastVLine(220, 60, 40, black);
          tft.drawFastVLine(120, 65, 30, black);
          tft.drawFastVLine(70, 70, 20, black);
          tft.drawFastVLine(170, 70, 20, black);
          tft.fillRect(map(bl, 0, 255, 20, 210), 60, 10, 40, darkgrey);
          tft.setCursor(getTextXCenter(6, 120, 2), 120);
          tft.print(F("Volume"));
          tft.drawFastHLine(20, 160, 200, black);
          tft.drawFastVLine(20, 140, 40, black);
          tft.drawFastVLine(220, 140, 40, black);
          tft.drawFastVLine(120, 145, 30, black);
          tft.drawFastVLine(70, 150, 20, black);
          tft.drawFastVLine(170, 150, 20, black);
          tft.fillRect(map(vol, 0, 100, 20, 210), 140, 10, 40, darkgrey);
          break;
        case 1:
          tft.setCursor(10, 30);
          tft.print(F("Battery: "));
          getBatt();
          tft.print(batt);
          tft.print('%');
          tft.setCursor(10, 60);
          tft.print("Charging: ");
          if (charging) {
            tft.print("Yes");
          } else {
            tft.print("No");
          }
          break;
        case 2:
          tft.setCursor(getTextXCenter(8, 70, 2), getTextYCenter(60, 2));
          tft.print("Airplane");
          tft.setCursor(getTextXCenter(4, 70, 2), getTextYCenter(80, 2));
          tft.print("Mode");
          if (airplane) {
            drawButton(140, 30, 80, 80, "ON", 2, darkgreen, green, false, true);
          } else {
            drawButton(140, 30, 80, 80, "OFF", 2, maroon, red, false, true);
          }
          break;
      }
      break;
    case SCREEN_CALC:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, orange);
      drawTRect(10, 30, 220, 60, black, 3);
      tft.setCursor(20, 40);
      tft.setTextSize(1);
      tft.setTextColor(black);
      tft.print("This ain't functional");
      drawNumpad(0, 130);
      break;
    default:
      tft.fillScreen(black);
      tft.setTextColor(white, black);
      tft.setTextSize(2);
      tft.setCursor(0, 0);
      tft.print(F("ERROR (app n. "));
      tft.print(screen);
      tft.print(F("):\nNot Found\n\nReturning to menu.."));
      delay(5000);
      screen = SCREEN_MENU;
      draw(SCREEN_MENU, false);
      break;
  }
  openApp = screen;
  updateTimer = -10001;
}

void drawKeypad(int x, int y) {
  tft.fillRect(x, y, NUMPAD_W, NUMPAD_H, lightgrey);
  drawTRect(x, y, NUMPAD_W, NUMPAD_H, black, 3);
  tft.setTextColor(black);
  tft.setTextSize(2);
  upperCase = false;
  int letrs;
  letrs = 0;
  for (i = 0; i < 12; i++) {
    tft.setCursor(getTextXCenter(3, x + kpX[i], 1), y + kpY[i]);
    for (a = 0; a < 3; a++) {
      tft.print(charsL[letrs]);
      Serial.print(charsL[letrs]);
      letrs++;
    }
    Serial.println();
  }
  keypadX = x;
  keypadY = y;
}

void drawNumpad(byte x, byte y) {
  tft.fillRect(x, y, NUMPAD_W, NUMPAD_H, lightgrey);
  drawTRect(x, y, NUMPAD_W, NUMPAD_H, black, 3);
  tft.setTextColor(black);
  tft.setTextSize(3);
  for (i = 0; i < 12; i++) {
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
  if (!rot) {
    TP.x = map(TP.x, 0, 240, 240, 0);
  }
  return TP.x;
}

int getY() {
  TP = ts.getPoint();
  if (!rot) {
    TP.y = map(TP.y, 0, 320, 320, 0);
  }
  return TP.y;
}

int getColor(int r, int g, int b) {
  return ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
}

/*int getTextLength(char text[], int fontS) {
  int length = getChars(text) * (6 * fontS);
  return length;
  }*/

int getChars(char text[]) {
  int chars;
  chars = 0;
  String tempText = String(text);
  for (int index = 0; index < tempText.length(); index++) {
    if (text[index] != '\0') {
      chars++;
    }
  }
  return chars;
}

int getTextXCenter(int length, int xCoord, int font) {
  //Serial.print(F("X: "));
  //Serial.println(xCoord - (length * (font * 6)) / 2);
  return xCoord - (length * (font * 6)) / 2;
}

int getTextYCenter(int yCoord, int font) {
  //Serial.print(F("Y: "));
  //Serial.println(yCoord - 3.5 * font);
  return yCoord - 3.5 * font;
}

void drawTRect(int x, int y, int w, int h, int color, int t) {
  for (int line = 0; line < t; line++) {
    tft.drawRect(x + line, y + line, w - line * 2, h - line * 2, color);
  }
}

bool homeBtn() {
  if (ts.touched()) {
    if (getY() < 30) {
      return true;
    }
  }
  return false;
}

bool incCall() {
  bool ringPin = digitalRead(FONA_RI);
  if (!ringPin && fona.getCallStatus() == 3) {
    Serial.println(F("OS: Incoming call!"));
    return true;
  } else {
    return false;
  }
}

bool newSMS() {
  bool ringPin = digitalRead(FONA_RI);
  if (!ringPin && fona.getCallStatus() != 3) {
    Serial.println(F("OS: SMS received!"));
    oldSMSNum = fona.getNumSMS();
    return true;
  } else {
    return false;
  }
}

void shutdown() {
  tft.fillScreen(black);

  EEPROM.write(1, vol);
  EEPROM.write(2, bl);
  EEPROM.write(3, aud);

  digitalWrite(FONA_KEY, LOW);
  delay(2100);
  digitalWrite(FONA_KEY, HIGH);

  tft.setTextSize(1);
  tft.setTextColor(white);
  tft.setCursor(getTextXCenter(21, 120, 1), getTextYCenter(160, 1));
  tft.print(F("You can now power off"));
  if (ts.touched()) {
    asm volatile ("jmp 0");
  }
  while (1);
}











