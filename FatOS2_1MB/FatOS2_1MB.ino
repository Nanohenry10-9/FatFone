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

int bl = 100;

char* appNames[] = {"Phone", "SMS", "Settings", "Contacts", "Calc", "Radio", "UNO", "PONG"};
int appColor[] = {red, green, darkgrey, blue, orange, navy, red, black};
//int appColor[] = {black, black, black, black, black, black, black, black};
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

char kpC[] = {'1', '2', '3',
              '4', '5', '6',
              '7', '8', '9',
              '+', '0', '#'
             };

byte kpX[] = {18,  61,  104,
              18,  61,  104,
              18,  61,  104,
              18,  61,  104
             };

byte kpY[] = {20,  20,  20,
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

char time[] = {"12:00"};
uint16_t batt = 100;

long updateTimer = 0;
long slideTimer = 0;

char phoneNumber[23] = {' '};
int phoneNumberIndex = 0;
char chistory[5][23] = {"123456789", "987654321", "123321123", "111222333", "147258369"};
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

char* settings[4] = {"Battery", "Screen", "Sounds", "Phone"};
byte selectedSet = -1;

char* chats[4] = {"Cont1", "Cont2", "Cont3", "Cont4"};
int newMsgs[4] = {false, false, false, false};
char msgs1[6][255] = {"Hello\0", "How are you?\0", "Good\0", "Alright\0"};
char msgs2[6][255] = {"Hello\0", "How are you?\0", "Good\0", "Alright\0"};
char msgs3[6][255] = {"Hello\0", "How are you?\0", "Good\0", "Alright\0"};
char msgs4[6][255] = {"Hello\0", "How are you?\0", "Good\0", "Alright\0"};
bool oMsg[4][6] = {{0, 0, 1, 0}, {0, 0, 0, 1}, {1, 0, 0, 0}, {0, 1, 0, 0}};
int ms1[4] = {4, 4, 4, 4};
int smsChat = -1;

File SDfile;

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
  if (!digitalRead(FONA_PWR)) {
    tft.print(F("Powering up FONA... "));
    digitalWrite(FONA_KEY, LOW);
    delay(2100); // Delay to get FONA started
    digitalWrite(FONA_KEY, HIGH);
    while (!digitalRead(FONA_PWR));
    tft.println(F("Done"));
  }
  tft.print(F("Starting FONA [1/2]... "));
  fonaSerial->begin(4800);
  tft.println(F("Done"));
  tft.print(F("Starting FONA [2/2]... "));
  fona.begin(*fonaSerial);
  tft.println(F("Done"));
  tft.print(F("Setting FONA settings... "));
  fona.setAudio(FONA_EXTAUDIO);
  fona.setAllVolumes(50);
  fona.callerIdNotification(true, digitalPinToInterrupt(3));
  fona.setPWM(0);
  tft.println(F("Done"));
  tft.print(F("Starting graphical UI..."));
  draw(SCREEN_LOCK, 0);
  /*tone(45, 600, 30);
    delay(40);
    tone(45, 600, 200);*/
}

void loop() {
  if (ts.touched()) {
    touchHandler(openApp);
  }

  if (millis() - updateTimer >= 10000) {
    appRoutine(openApp);
    updateTimer = millis();
  }

  if (homeBtn() && openApp != SCREEN_MENU && openApp != SCREEN_LOCK) {
    draw(SCREEN_MENU, true);
  }

  if (homeBtn() && openApp == SCREEN_MENU) {
    draw(SCREEN_HOME, true);
  }

  if (!digitalRead(FONA_RI) && fona.getCallStatus() == 3 && openApp != SCREEN_LOCK) {
    Serial.println("Incoming call!");
    bool answered = callFrom();
    if (!answered && openApp != SCREEN_CHIS) {
      popup("Missed call", "View", PHONE_CALL);
    }
  }

  if (!digitalRead(FONA_RI) && fona.getCallStatus() != 3 && openApp != SCREEN_LOCK && openApp != SCREEN_SMS2) {
    Serial.println("SMS received!");
    popup("New SMS received", "View", SMS_MSG);
  }

  if (backBtn()) {
    switch (openApp) {
      case SCREEN_SMS2:
        draw(SCREEN_SMS1, false);
        break;
      case SCREEN_SMSS:
        draw(SCREEN_SMS2, false);
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
        } else if (inputpw != "    ") {
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
          for (i = 0; i < 4; i++) {
            inputpw[i] = ' ';
          }
          inputpwindex = 0;
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
          tft.print(phoneNumber[21]);
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
          tft.setCursor(getTextCenter("abc", kpX[11], 1), 130 + kpY[11]);
          for (i = 33; i < 36; i++) {
            tft.print(charsL[i]);
          }
          buzz(5);

          tft.setTextColor(black, lightgrey);
          int letrs = 0;
          for (i = 0; i < 11; i++) {
            tft.setCursor(getTextCenter("abc", kpX[i], 1), 130 + kpY[i]);
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
          // Redraw the keypad white
        }
      }
      if (tX >= NUMPAD_W && tX <= NUMPAD_W + 100 && tY >= 130 && tY <= 193) {
        drawButton(NUMPAD_W, 130, 100, 63, "SEND", 3, darkgreen, green, true, false);
        while (ts.touched());
        // Send SMS (FONA)
        for (i = 0; i < maxSMS; i++) {
          msgs1[ms1[smsChat]][i] = smsMsg[i];
        }
        oMsg[smsChat][ms1[smsChat]] = true;
        if (ms1[smsChat] < 5) {
          ms1[smsChat]++;
        }
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
        // Option 1
      }
      if (tY > 90 && tY <= 160) {
        // Option 2
      }
      if (tY > 160 && tY <= 230) {
        // Option 3
      }
      if (tY > 230 && tY <= 300) {
        // Option 4
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
  tft.setCursor(getTextCenter(msg, 120, 2), getTextCenter(70, 2));
  tft.print(msg);
  tft.setTextSize(3);
  tft.setCursor(getTextCenter(btn1, 120, 3), getTextCenter(145, 3));
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
    for (a = 0; a < 23; a++) {
      chistory[i][a] = chistory[i - 1][a];
    }
    answered[i] = answered[i - 1];
    called[i] = called[i - 1];
  }
  for (i = 0; i < 22; i++) {
    //if (isDigit(tonumber[i])) {
    chistory[0][i] = tonumber[i + 1];
    //}
  }
  answered[0] = 1;

  for (i = 0; i < 30; i++) {
    phoneNumber[i] = ' ';
    phoneNumberIndex = 0;
  }
  draw(SCREEN_PHONE, true);
}

bool callFrom() {
  bool ans = 0;
  char incNumber[22];
  tft.fillScreen(darkgrey);
  tft.setTextSize(2);
  tft.setTextColor(white);
  tft.setCursor(getTextCenter("Incoming call...", 120, 2), getTextCenter(40, 2));
  tft.print("Incoming call...");

  drawButton(20, 140, 200, 70, "ANSWER", 3, darkgreen, green, false, true);
  drawButton(20, 230, 200, 70, "END CALL", 3, maroon, red, false, true);

  fona.setPWM(2000);

  tft.setTextSize(2);
  tft.setCursor(20, 70);
  tft.setTextColor(white, darkgrey);
  tft.print(F("Loading number.."));
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
  for (i = 10; i < 21; i++) {
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
    for (a = 0; a < 23; a++) {
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
      tft.setCursor(getTextCenter(inputpw, 120, 4), getTextCenter(65, 4));
      tft.setTextColor(red, white);
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
      tft.setCursor(getTextCenter("Swipe left or right for call history", 120, 1), getTextCenter(115, 1));
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
        tft.setCursor(2, (i + 1) * 60 - 30);
        tft.print(F("+"));
        for (a = 0; a < 12; a++) {
          tft.print(chistory[i][a]);
        }
        tft.setCursor(20, (i + 1) * 60 - 5);
        for (a = 12; a < 22; a++) {
          tft.print(chistory[i][a]);
        }
      }
      break;
    case SCREEN_SMS1:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, green);
      tft.setTextColor(black);
      a = 0;
      for (i = 90; i < 320; i += 70) {
        tft.drawFastHLine(0, i, 240, darkgrey);
        tft.setTextSize(3);
        tft.setCursor(20, getTextCenter(i - 35, 3));
        if (getChars(chats[a]) > 9) {
          tft.setTextSize(2);
          tft.setCursor(20, getTextCenter(i - 35, 2));
        }
        tft.print(chats[a]);
        a++;
      }
      break;
    case SCREEN_SMS2:
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, green);
      tft.setTextSize(2);
      tft.setTextColor(black);
      for (a = 0; a < ms1[smsChat]; a++) {
        tft.setCursor(10, (a + 1) * 40);
        if (oMsg[smsChat][a]) {
          tft.print("You: ");
          Serial.print("You: ");
        } else {
          tft.print(chats[smsChat]);
          Serial.print(chats[smsChat]);
          tft.print(": ");
          Serial.print(": ");
        }
        tft.print(msgs1[a]);
        Serial.println(msgs1[a]);
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
        tft.setCursor(20, getTextCenter(i - 35, 3));
        tft.print(settings[a]);
        a++;
      }
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
    tft.setCursor(getTextCenter("abc", x + kpX[i], 1), y + kpY[i]);
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

bool backBtn() {
  return !digitalRead(19);
}











