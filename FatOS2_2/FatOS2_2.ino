// Sketch needs an AVR mC with >80KB of flash!

#include <Adafruit_ILI9341.h>
#include <Adafruit_FONA.h>
#include <Adafruit_FT6206.h>
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
#define FONA_RST 0
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

#define MENU          0
#define APP_PHONE     1
#define APP_SMS       2
#define APP_SETTINGS  3
#define APP_PONG      4
#define APP_RADIO     5
#define APP_CLOCK     6
#define APP_PAINT     7
#define APP_CALC      8
#define APP_CONTACTS  9

#define KEYPAD_CHARS  -2
#define KEYPAD_NUMS   -1

#define CALC_PLUS 0
#define CALC_MINUS 1
#define CALC_MULT 2
#define CALC_DIV 3

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

bool noFONA = false;
bool noSD = false;

bool hasStarted = false;

#define appSize 60
byte appLocX[] = {10, 90, 170, 10, 90, 170, 10, 90, 170};
byte appLocY[] = {60, 60, 60, 140, 140, 140, 220, 220, 220};
uint16_t appColor[] = {red, green, lightgrey, black, navy, darkgrey, maroon, orange, navy};
char* appName[] = {"Phone", "Messages", "Settings", "Pong", "Radio", "Clock", "Paint", "Calc", "Contacts"};
byte appNamePlusX[] = {15, 7, 7, 17, 15, 15, 15, 17, 7};
byte appAmount = 9;
#define appAnimFrames 71

char errorFONA[] = {"!FONA"};

byte bl = 5;
byte audio = FONA_EXTAUDIO;
byte volume = 5;

char RTCtime[23];

long updateTimer = 20000;

byte appOnScreen = 0;

bool appExit = false;

char givenPNumber[14] = {' '};
int mesLocY[] = {100, 160, 220, 280};
int pNumCount = 0;

char chars[] = "aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ.!? ";
char message[21] = {' '};
int charOnScreenNum = 0;
int messageIndex = 0;

uint16_t channelNums[5] = {0, 0, 0, 0};
uint16_t channelNum = 0000;

int clockToX[] = {120};
int clockToY[] = {80};

int x;
int y;
TS_Point tPoint;

int blToA;

String memos[3] = {{"\n"}, {"\n"}, {"\n"}};

#define BTN_BACK 19
#define BTN_HOME 18
long int btnTimer = 0;

int battPercent = 0;
int netState = 0;

File SDfile;

long netTimeout = 0;
long lockTimer = 0;
int lockCount = 0;
int lockCycles = 12;

bool doDemo = false;

void setup() {
  ts.begin(40);
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  pinMode(BTN_HOME, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  if (!SD.begin(SD_CS)) {
    noSD = true;
  }
  tft.begin();
  tft.setRotation(0);
  tft.setTextWrap(false);
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

  if (!noSD) {

    Serial.println(F("Reading settings from SD..."));
    Serial.print(F("audio: "));
    SDfile = SD.open("AUD_TYP.txt");
    audio = SDfile.read();
    SDfile.close();
    audio = audio - '0';
    Serial.print(audio);
    Serial.print(F(", vol: "));

    SDfile = SD.open("SET_VOL.txt");
    volume = SDfile.read();
    SDfile.close();
    volume = volume - '0';
    Serial.print(volume);
    Serial.print(F(", bl: "));

    SDfile = SD.open("SET_BL.txt");
    bl = SDfile.read();
    SDfile.close();
    bl = bl - '0';
    Serial.println(bl);

  } else {
    drawText("NO SD CARD", 65, 250, 2, red, navy);
  }

  if (bl > 10) {
    bl = 10;
  }
  if (volume > 10) {
    volume = 10;
  }
  if (audio > 1) {
    audio = 1;
  }

  blToA = map(bl, 0, 10, 0, 255);
  for (int i = 0; i < blToA; i++) {
    analogWrite(TFT_BL, i);
    delay(10);
  }
  if (digitalRead(BTN_BACK) == 0 && digitalRead(BTN_HOME) == 0) {
    noFONA = true;
    drawText("FONA BYPASSED", 43, 250, 2, green, navy);
  } else {
    tft.setCursor(85, 200);
    tft.setTextColor(white, navy);
    tft.setTextSize(3);
    Serial.print(F("Connecting to FONA with (RX, TX): "));
    Serial.print(FONA_RX);
    Serial.print(F(", "));
    Serial.println(FONA_TX);
    Serial.println(F("Starting FONA 1/2..."));
    fonaSerial->begin(4800);
    tft.print('.');
    Serial.println(F("Starting FONA 2/2..."));
    if (!fona.begin(*fonaSerial)) {
      Serial.println(F("Couldn't start FONA"));
      drawText("FONA ERROR", 63, 250, 2, red, navy);
      while (true) {}
    }
    Serial.println(F("FONA started"));
    tft.print('.');
    fona.setAudio(audio);
    tft.print('.');
    fona.setPWM(2000);
    fona.setAllVolumes(volume * 10);
    tft.print(F("."));
    fona.setPWM(0);
    netState = fona.getNetworkStatus();
    if (netState == 1) {
      drawText("NETWORK FOUND", 45, 250, 2, white, navy);
    } else {
      drawText("FINDING NETWORK", 30, 250, 2, orange, navy);
      netTimeout = millis();
      while (netState != 1 && millis() - netTimeout < 10000) {
        netState = fona.getNetworkStatus();
      }
      if (netState != 1) {
        tft.fillRect(20, 230, 220, 50, navy);
        drawText("NO NETWORK", 60, 250, 2, red, navy);
      } else if (netState == 1) {
        tft.fillRect(20, 230, 220, 50, navy);
        drawText("NETWORK FOUND", 45, 250, 2, white, navy);
      }
    }
    delay(1000);
  }

  hasStarted = true;

  exitApp(); // Isn't actually exiting app, here just for the animation
  lockTimer = millis();
}

void loop() {
  if (millis() - updateTimer >= 20000) {
    drawTime(blue);
    drawBattery();
    updateTimer = millis();
  }
  if (ts.touched()) {
    lockTimer = millis();
    lockCount = 0;
    touchHandler();
  }
  if (millis() - lockTimer >= 5000) {
    lockCount++;
    if (lockCount >= lockCycles) {
      lockCount = 0;
      if (doDemo) {
        demo();
      } else {
        lock();
      }
      lockTimer = millis();
    }
    lockTimer = millis();
  }
  if (digitalRead(BTN_BACK) == 0 && digitalRead(BTN_HOME) == 0) {
    btnTimer = millis();
    while (digitalRead(BTN_BACK) == 0 && digitalRead(BTN_HOME) == 0) {}
    if (millis() - btnTimer > 6000) {
      tft.fillRect(20, 100, 200, 100, white);
      tft.drawRect(20, 100, 200, 100, black);
      tft.setTextColor(black, white);
      tft.setTextSize(3);
      tft.setCursor(40, 120);
      tft.print(F("Restart?"));
      tft.setCursor(40, 160);
      tft.print(F("YES    NO"));
      while (digitalRead(BTN_BACK) == 1 && digitalRead(BTN_HOME) == 1) {}
      if (digitalRead(BTN_BACK) == 0) {
        asm volatile ("jmp 0");
      }
      exitApp();
    }
  }
}

void draw(int a) {
  switch (a) {
    case MENU:
      for (byte a = 0; a < appAmount; a++) {
        tft.fillRect(appLocX[a], appLocY[a], appSize, appSize, appColor[a]);
        drawText(appName[a], (appLocX[a] + appNamePlusX[a]), (appLocY[a] + 26), 1, white, appColor[a]);
      }
      tft.fillRect(7, 5, 40, 40, red);
      tft.drawRect(7, 5, 40, 40, black);
      drawTime(blue);
      drawBattery();
      updateTimer = millis();
      break;
    case APP_PHONE:
      drawTime(appColor[0]);
      drawBattery();
      tft.fillRect(20, 70, 90, 50, green);
      drawText("PICK UP", 25, 87, 2, white, green);
      tft.fillRect(130, 70, 90, 50, red);
      drawText("END", 157, 87, 2, white, red);
      tft.fillRect(20, 140, 200, 80, lightgrey);
      drawText("KEYPAD", 65, 168, 3, black, lightgrey);
      break;
    case APP_SMS:
      drawTime(appColor[1]);
      drawBattery();
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
    case APP_SETTINGS:
      drawTime(appColor[2]);
      drawBattery();
      drawText("BACKLIGHT", 65, 70, 2, black, white);
      tft.drawFastHLine(20, 110, 200, black);
      tft.drawFastVLine(20, 90, 40, black);
      tft.drawFastVLine(220, 90, 40, black);
      tft.drawFastVLine(120, 95, 30, black);
      tft.drawFastVLine(70, 100, 20, black);
      tft.drawFastVLine(170, 100, 20, black);
      tft.fillRect(map(bl, 1, 10, 20, 210), 90, 10, 40, darkgrey);

      drawText("VOLUME", 85, 150, 2, black, white);
      tft.drawFastHLine(20, 190, 200, black);
      tft.drawFastVLine(20, 170, 40, black);
      tft.drawFastVLine(220, 170, 40, black);
      tft.drawFastVLine(120, 175, 30, black);
      tft.drawFastVLine(70, 180, 20, black);
      tft.drawFastVLine(170, 180, 20, black);
      tft.fillRect(map(volume, 1, 10, 20, 210), 170, 10, 40, darkgrey);

      drawText("AUDIO OUT", 65, 230, 2, black, white);
      tft.fillRect(30, 260, 180, 50, lightgrey);
      tft.drawRect(30, 260, 180, 50, black);
      tft.drawFastVLine(120, 260, 50, black);
      tft.setTextSize(1);
      tft.setTextColor(black, lightgrey);
      tft.setCursor(45, 285);
      tft.print(F("LOUDSPEAKER"));
      tft.setCursor(135, 285);
      tft.print(F("HEADPHONES"));
      break;
    case APP_PONG:
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
      break;
    case APP_RADIO:
      drawTime(appColor[4]);
      drawBattery();
      tft.fillRect(30, 190, 180, 40, green);
      tft.fillRect(30, 240, 180, 40, darkgrey);
      drawText("TUNE", 85, 200, 3, white, green);
      drawText("CLOSE", 75, 250, 3, white, darkgrey);
      tft.fillRect(20, 90, 200, 50, white);
      tft.drawRect(20, 90, 200, 50, black);

      tft.fillTriangle(20, 80, 40, 60, 60, 80, red);
      tft.fillTriangle(20, 150, 40, 170, 60, 150, red);
      tft.fillTriangle(70, 80, 90, 60, 110, 80, red);
      tft.fillTriangle(70, 150, 90, 170, 110, 150, red);

      tft.fillTriangle(120, 80, 140, 60, 160, 80, red);
      tft.fillTriangle(120, 150, 140, 170, 160, 150, red);
      tft.fillTriangle(180, 80, 200, 60, 220, 80, red);
      tft.fillTriangle(180, 150, 200, 170, 220, 150, red);

      tft.setTextSize(4);
      tft.setTextColor(black, white);
      tft.setCursor(30, 100);
      tft.print(channelNums[0]);
      tft.setCursor(80, 100);
      tft.print(channelNums[1]);
      tft.setCursor(130, 100);
      tft.print(channelNums[2]);
      tft.setCursor(160, 100);
      tft.print(F("."));
      tft.setCursor(190, 100);
      tft.print(channelNums[3]);
      break;
    case APP_CLOCK:
      drawTime(appColor[5]);
      drawBattery();
      tft.drawCircle(120, 160, 101, black);
      tft.drawCircle(120, 160, 102, black);
      break;
    case APP_PAINT:
      drawTime(appColor[6]);
      drawBattery();
      tft.fillRect(0, 280, 240, 40, darkgrey);
      tft.fillRect(5, 285, 38, 30, red);
      tft.fillRect(43, 285, 38, 30, green);
      tft.fillRect(81, 285, 38, 30, blue);
      tft.fillRect(119, 285, 38, 30, black);
      tft.fillRect(157, 285, 38, 30, white);
      tft.fillRect(195, 285, 38, 30, white);
      tft.drawFastVLine(195, 285, 30, black);
      tft.drawRect(162, 290, 28, 20, black);
      tft.drawFastVLine(170, 290, 20, black);
      tft.fillRect(171, 291, 18, 18, blue);
      tft.drawFastVLine(214, 290, 20, black);
      tft.drawFastHLine(204, 300, 20, black);
      break;
    case APP_CALC:
      drawTime(appColor[7]);
      drawBattery();
      tft.drawRect(20, 60, 200, 40, black);
      tft.fillRect(20, 110, 200, 40, orange);
      tft.fillRect(20, 160, 200, 40, orange);
      tft.fillRect(20, 210, 200, 40, orange);
      tft.fillRect(20, 260, 200, 40, orange);

      tft.drawFastVLine(70, 110, 40, black);
      tft.drawFastVLine(120, 110, 40, black);
      tft.drawFastVLine(170, 110, 40, black);

      tft.drawFastVLine(70, 160, 40, black);
      tft.drawFastVLine(120, 160, 40, black);
      tft.drawFastVLine(170, 160, 40, black);

      tft.drawFastVLine(70, 210, 40, black);
      tft.drawFastVLine(120, 210, 40, black);
      tft.drawFastVLine(170, 210, 40, black);

      tft.drawFastVLine(70, 260, 40, black);
      tft.drawFastVLine(120, 260, 40, black);
      tft.drawFastVLine(170, 260, 40, black);

      tft.setTextColor(black, orange);
      tft.setTextSize(3);
      tft.setCursor(35, 120);
      tft.print(1);
      tft.setCursor(85, 120);
      tft.print(2);
      tft.setCursor(135, 120);
      tft.print(3);
      tft.setCursor(185, 120);
      tft.print(4);

      tft.setCursor(35, 170);
      tft.print(5);
      tft.setCursor(85, 170);
      tft.print(6);
      tft.setCursor(135, 170);
      tft.print(7);
      tft.setCursor(185, 170);
      tft.print(8);

      tft.setCursor(35, 220);
      tft.print(9);
      tft.setCursor(85, 220);
      tft.print(0);
      tft.setCursor(135, 220);
      tft.print('C');
      tft.setCursor(185, 220);
      tft.print('=');

      tft.setCursor(35, 270);
      tft.print('+');
      tft.setCursor(85, 270);
      tft.print('-');
      tft.setCursor(135, 270);
      tft.print('*');
      tft.setCursor(185, 270);
      tft.print('/');
      break;
    case APP_CONTACTS:
      drawTime(appColor[8]);
      drawBattery();
      break;
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
      for (int i = 0; i < appAnimFrames; i++) {
        if (i % 8 == 0 || i == 70) {
          y = map(i, 0, 70, appLocY[0], 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 12, cyan);
          }
          x = map(i, 0, 70, appLocX[0], 0);
          sizeX = map(i, 0, 70, appSize, 240);
          sizeY = map(i, 0, 70, appSize, 50);
          tft.fillRect(x, y, sizeX, sizeY, appColor[0]);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      phoneApp();
      break;
    case APP_SMS:
      for (int i = 0; i < appAnimFrames; i++) {
        if (i % 8 == 0 || i == 70) {
          y = map(i, 0, 70, appLocY[1], 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 12, cyan);
          }
          x = map(i, 0, 70, appLocX[1], 0);
          sizeX = map(i, 0, 70, appSize, 240);
          sizeY = map(i, 0, 70, appSize, 50);
          tft.fillRect(x, y, sizeX, sizeY, appColor[1]);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      smsApp();
      break;
    case APP_SETTINGS:
      for (int i = 0; i < appAnimFrames; i++) {
        if (i % 8 == 0 || i == 70) {
          y = map(i, 0, 70, appLocY[2], 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 20, cyan);
          }
          x = map(i, 0, 70, appLocX[2], 0);
          sizeX = map(i, 0, 70, appSize, 240);
          sizeY = map(i, 0, 70, appSize, 50);
          tft.fillRect(x, y, sizeX, sizeY, appColor[2]);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      setApp();
      break;
    case APP_PONG:
      for (int i = 0; i < appAnimFrames; i++) {
        if (i % 6 == 0 || i == 70) {
          y = map(i, 0, 70, appLocY[3], 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 20, cyan);
          }
          x = map(i, 0, 70, appLocX[3], 0);
          sizeX = map(i, 0, 70, appSize, 240);
          sizeY = map(i, 0, 70, appSize, 50);
          tft.fillRect(x, y, sizeX, sizeY, appColor[3]);
        }
      }
      tft.fillRect(0, 50, 240, 270, black);
      pongApp();
      break;
    case APP_RADIO:
      for (int i = 0; i < appAnimFrames; i++) {
        if (i % 6 == 0 || i == 70) {
          y = map(i, 0, 70, appLocY[4], 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 20, cyan);
          }
          x = map(i, 0, 70, appLocX[4], 0);
          sizeX = map(i, 0, 70, appSize, 240);
          sizeY = map(i, 0, 70, appSize, 50);
          tft.fillRect(x, y, sizeX, sizeY, appColor[4]);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      radioApp();
      break;
    case APP_CLOCK:
      for (int i = 0; i < appAnimFrames; i++) {
        if (i % 6 == 0 || i == 70) {
          y = map(i, 0, 70, appLocY[5], 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 20, cyan);
          }
          x = map(i, 0, 70, appLocX[5], 0);
          sizeX = map(i, 0, 70, appSize, 240);
          sizeY = map(i, 0, 70, appSize, 50);
          tft.fillRect(x, y, sizeX, sizeY, appColor[5]);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      clockApp();
      break;
    case APP_PAINT:
      for (int i = 0; i < appAnimFrames; i++) {
        if (i % 6 == 0 || i == 70) {
          y = map(i, 0, 70, appLocY[6], 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 20, cyan);
          }
          x = map(i, 0, 70, appLocX[6], 0);
          sizeX = map(i, 0, 70, appSize, 240);
          sizeY = map(i, 0, 70, appSize, 50);
          tft.fillRect(x, y, sizeX, sizeY, appColor[6]);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      paintApp();
      break;
    case APP_CALC:
      for (int i = 0; i < appAnimFrames; i++) {
        if (i % 6 == 0 || i == 70) {
          y = map(i, 0, 70, appLocY[7], 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 20, cyan);
          }
          x = map(i, 0, 70, appLocX[7], 0);
          sizeX = map(i, 0, 70, appSize, 240);
          sizeY = map(i, 0, 70, appSize, 50);
          tft.fillRect(x, y, sizeX, sizeY, appColor[7]);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      calcApp();
      break;
    case APP_CONTACTS:
      for (int i = 0; i < appAnimFrames; i++) {
        if (i % 6 == 0 || i == 70) {
          y = map(i, 0, 70, appLocY[8], 0);
          if (i != 0) {
            tft.fillRect(x, (y + sizeY), sizeX, 20, cyan);
          }
          x = map(i, 0, 70, appLocX[8], 0);
          sizeX = map(i, 0, 70, appSize, 240);
          sizeY = map(i, 0, 70, appSize, 50);
          tft.fillRect(x, y, sizeX, sizeY, appColor[8]);
        }
      }
      tft.fillRect(0, 50, 240, 270, white);
      contApp();
      break;
  }
}

void drawText(char* text, byte locX, byte locY, byte textSize, uint16_t color, uint16_t bgcolor) {
  tft.setCursor(locX, locY);
  tft.setTextColor(color, bgcolor);
  tft.setTextSize(textSize);
  tft.print(text);
}

void backlight(int a) {
  int b = map(a, 0, 10, 0, 255);
  analogWrite(TFT_BL, b);
}

void touchHandler() {
  tPoint = ts.getPoint();
  x = map(tPoint.x, 0, 240, 240, 0);
  y = map(tPoint.y, 0, 320, 320, 0);
  if (x >= 7 && y >= 5 && x <= 47 && y <= 45) {
    tft.fillRect(8, 6, 38, 38, maroon);
    while (ts.touched()) {}
    tft.fillRect(8, 6, 38, 38, red);
    lock();
  } else if (x >= appLocX[0] && y >= appLocY[0] && x <= (appLocX[0] + appSize) && y <= (appLocY[0] + appSize)) {
    drawText(appName[0], (appLocX[0] + appNamePlusX[0]), (appLocY[0] + 26), 1, white, appColor[0]);
    while (ts.touched()) {}
    drawText(appName[0], (appLocX[0] + appNamePlusX[0]), (appLocY[0] + 26), 1, black, appColor[0]);
    appOnScreen = APP_PHONE;
    openApp(APP_PHONE);
  } else if (x >= appLocX[1] && y >= appLocY[1] && x <= (appLocX[1] + appSize) && y <= (appLocY[1] + appSize)) {
    drawText(appName[1], (appLocX[1] + appNamePlusX[1]), (appLocY[1] + 26), 1, white, appColor[1]);
    while (ts.touched()) {}
    drawText(appName[1], (appLocX[1] + appNamePlusX[1]), (appLocY[1] + 26), 1, black, appColor[1]);
    appOnScreen = APP_SMS;
    openApp(APP_SMS);
  } else if (x >= appLocX[2] && y >= appLocY[2] && x <= (appLocX[2] + appSize) && y <= (appLocY[2] + appSize)) {
    drawText(appName[2], (appLocX[2] + appNamePlusX[2]), (appLocY[2] + 26), 1, white, appColor[2]);
    while (ts.touched()) {}
    drawText(appName[2], (appLocX[2] + appNamePlusX[2]), (appLocY[2] + 26), 1, black, appColor[2]);
    appOnScreen = APP_SETTINGS;
    openApp(APP_SETTINGS);
  } else if (x >= appLocX[3] && y >= appLocY[3] && x <= (appLocX[3] + appSize) && y <= (appLocY[3] + appSize)) {
    drawText(appName[3], (appLocX[3] + appNamePlusX[3]), (appLocY[3] + 26), 1, white, appColor[3]);
    while (ts.touched()) {}
    drawText(appName[3], (appLocX[3] + appNamePlusX[3]), (appLocY[3] + 26), 1, black, appColor[3]);
    appOnScreen = APP_PONG;
    openApp(APP_PONG);
  } else if (x >= appLocX[4] && y >= appLocY[4] && x <= (appLocX[4] + appSize) && y <= (appLocY[4] + appSize)) {
    drawText(appName[4], (appLocX[4] + appNamePlusX[4]), (appLocY[4] + 26), 1, white, appColor[4]);
    while (ts.touched()) {}
    drawText(appName[4], (appLocX[4] + appNamePlusX[4]), (appLocY[4] + 26), 1, black, appColor[4]);
    appOnScreen = APP_RADIO;
    openApp(APP_RADIO);
  } else if (x >= appLocX[5] && y >= appLocY[5] && x <= (appLocX[5] + appSize) && y <= (appLocY[5] + appSize)) {
    drawText(appName[5], (appLocX[5] + appNamePlusX[5]), (appLocY[5] + 26), 1, white, appColor[5]);
    while (ts.touched()) {}
    drawText(appName[5], (appLocX[5] + appNamePlusX[5]), (appLocY[5] + 26), 1, black, appColor[5]);
    appOnScreen = APP_CLOCK;
    openApp(APP_CLOCK);
  } else if (x >= appLocX[6] && y >= appLocY[6] && x <= (appLocX[6] + appSize) && y <= (appLocY[6] + appSize)) {
    drawText(appName[6], (appLocX[6] + appNamePlusX[6]), (appLocY[6] + 26), 1, white, appColor[6]);
    while (ts.touched()) {}
    drawText(appName[6], (appLocX[6] + appNamePlusX[6]), (appLocY[6] + 26), 1, black, appColor[6]);
    appOnScreen = APP_PAINT;
    openApp(APP_PAINT);
  } else if (x >= appLocX[7] && y >= appLocY[7] && x <= (appLocX[7] + appSize) && y <= (appLocY[7] + appSize)) {
    drawText(appName[7], (appLocX[7] + appNamePlusX[7]), (appLocY[7] + 26), 1, white, appColor[7]);
    while (ts.touched()) {}
    drawText(appName[7], (appLocX[7] + appNamePlusX[7]), (appLocY[7] + 26), 1, black, appColor[7]);
    appOnScreen = APP_CALC;
    openApp(APP_CALC);
  } else if (x >= appLocX[8] && y >= appLocY[8] && x <= (appLocX[8] + appSize) && y <= (appLocY[8] + appSize)) {
    drawText(appName[8], (appLocX[8] + appNamePlusX[8]), (appLocY[8] + 26), 1, white, appColor[8]);
    while (ts.touched()) {}
    drawText(appName[8], (appLocX[8] + appNamePlusX[8]), (appLocY[8] + 26), 1, black, appColor[8]);
    appOnScreen = APP_CONTACTS;
    openApp(APP_CONTACTS);
  } else {
    if (debug) {
      tft.invertDisplay(true);
      while (ts.touched()) {}
      tft.invertDisplay(false);
    }
  }
}

void drawBattery() {
  tft.drawRect(batteryX, batteryY, 15, 40, black);
  tft.drawRect((batteryX + 1), (batteryY + 1), 13, 38, black);
  tft.fillRect((batteryX + 2), (batteryY + 2), 11, 36, white);
  tft.drawFastHLine((batteryX + 5), (batteryY - 2), 5, black);
  tft.drawFastHLine((batteryX + 5), (batteryY - 1), 5, black);
  battPercent = getBattery();
  if (battPercent <= 10) {
    tft.fillRect((batteryX + 2), (batteryY + 36), 11, 2, red);
  } else if (battPercent <= 20) {
    tft.fillRect((batteryX + 2), (batteryY + 30), 11, 8, yellow);
  } else if (battPercent <= 40) {
    tft.fillRect((batteryX + 2), (batteryY + 22), 11, 16, green);
  } else if (battPercent <= 60) {
    tft.fillRect((batteryX + 2), (batteryY + 14), 11, 24, green);
  } else if (battPercent <= 80) {
    tft.fillRect((batteryX + 2), (batteryY + 6), 11, 32, green);
  } else if (battPercent <= 100) {
    tft.fillRect((batteryX + 2), (batteryY + 2), 11, 36, green);
  }
}

byte getBattery() {
  uint16_t bat;
  if (noFONA) {
    bat = random(0, 100);
  } else {
    fona.getBattPercent(&bat);
  }
  return bat;
}

void drawTime(uint16_t bgcolor) {
  if (noFONA) {
    for (int i = 10; i < 15; i++) {
      RTCtime[i] = errorFONA[i - 10];
    }
  } else {
    fona.getTime(RTCtime, 23);
  }
  if (bgcolor != blue && bgcolor != cyan) {
    tft.setCursor(timeX - 13, timeY);
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
  while (appExit == false) {
    if (millis() - updateTimer >= 20000) {
      drawTime(appColor[0]);
      drawBattery();
      updateTimer = millis();
    }
    if (digitalRead(BTN_BACK) == 0 && page == 1) {
      page = 0;
      tft.fillRect(0, 50, 240, 270, white);
      tft.fillRect(20, 70, 90, 50, green);
      drawText("PICK UP", 25, 87, 2, white, green);
      tft.fillRect(130, 70, 90, 50, red);
      drawText("END", 157, 87, 2, white, red);
      tft.fillRect(20, 140, 200, 80, lightgrey);
      drawText("KEYPAD", 65, 168, 3, black, lightgrey);
    }
    if (digitalRead(BTN_HOME) == 0) {
      appExit = true;
      while (digitalRead(BTN_HOME) == LOW) {}
    }
    if (ts.touched()) {
      tPoint = ts.getPoint();
      x = map(tPoint.x, 0, 240, 240, 0);
      y = map(tPoint.y, 0, 320, 320, 0);
      if (page == 0) {
        if (x >= 20 && y >= 70 && x <= 110 && y <= 120) {
          drawText("PICK UP", 25, 87, 2, black, green);
          while (ts.touched()) {}
          if (!noFONA) {
            fona.pickUp();
          }
          drawText("PICK UP", 25, 87, 2, white, green);
        } else if (x >= 130 && y >= 70 && x <= 220 && y <= 120) {
          drawText("END", 157, 87, 2, black, red);
          while (ts.touched()) {}
          if (!noFONA) {
            fona.hangUp();
          }
          drawText("END", 157, 87, 2, white, red);
        } else if (x >= 20 && y >= 140 && x <= 220 && y <= 220) {
          drawText("KEYPAD", 65, 168, 3, white, lightgrey);
          while (ts.touched()) {}
          drawText("KEYPAD", 65, 168, 3, black, lightgrey);
          page = 1;
          tft.fillRect(0, 50, 240, 270, white);
          tft.drawRect(10, 60, 220, 50, black);
          tft.drawRect(11, 61, 218, 48, black);
          tft.fillRect(20, 115, 200, 30, green);
          drawText("CALL", 90, 120, 3, white, green);
          tft.fillRect(20, 155, 200, 30, lightgrey);
          drawText("CLEAR", 80, 160, 3, black, lightgrey);
          draw(KEYPAD_NUMS);
          tft.setTextSize(3);
          tft.setTextColor(black, white);
          tft.setCursor(17, 75);
          tft.print(givenPNumber);
          tft.drawRect(10, 60, 220, 50, black);
          tft.drawRect(11, 61, 218, 48, black);
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
        } else if (x >= 20 && y >= 115 && x <= 220 && y <= 145) {
          drawText("CALL", 90, 120, 3, black, green);
          while (ts.touched()) {}
          if (!noFONA) {
            fona.callPhone(givenPNumber);
          }
          for (int i = 0; i < 14; i++) {
            givenPNumber[i] = ' ';
          }
          pNumCount = 0;
          page = 0;
          tft.fillRect(0, 50, 240, 270, white);
          tft.fillRect(20, 70, 90, 50, green);
          drawText("PICK UP", 25, 87, 2, white, green);
          tft.fillRect(130, 70, 90, 50, red);
          drawText("END", 157, 87, 2, white, red);
          tft.fillRect(20, 140, 200, 80, lightgrey);
          drawText("KEYPAD", 65, 168, 3, black, lightgrey);
        } else if (x >= 20 && y >= 155 && x <= 220 && y <= 185) {
          drawText("CLEAR", 80, 160, 3, white, lightgrey);
          while (ts.touched()) {}
          drawText("CLEAR", 80, 160, 3, black, lightgrey);
          for (int i = 0; i < 14; i++) {
            givenPNumber[i] = ' ';
          }
          pNumCount = 0;
          tft.setTextSize(3);
          tft.setTextColor(black, white);
          tft.setCursor(17, 75);
          tft.print(givenPNumber);
          tft.drawRect(10, 60, 220, 50, black);
          tft.drawRect(11, 61, 218, 48, black);
        }
        if (y >= 190) {
          tft.setTextSize(3);
          tft.setTextColor(black, white);
          tft.setCursor(17, 75);
          tft.print(givenPNumber);
          tft.drawRect(10, 60, 220, 50, black);
          tft.drawRect(11, 61, 218, 48, black);
        }
      }
    }
    while (ts.touched()) {}
  }
  appExit = false;
  for (int i = 0; i < 9; i++) {
    givenPNumber[i] = ' ';
  }
  exitApp();
}

void smsApp() {
  draw(APP_SMS);
  byte page = SMS_SEND;
  byte selectedField = NUM_FIELD;
  char charOnScreen = chars[charOnScreenNum];
  while (appExit == false) {
    if (digitalRead(BTN_HOME) == LOW) {
      appExit = true;
      while (digitalRead(BTN_HOME) == LOW) {}
    }
    if (millis() - updateTimer >= 20000) {
      drawTime(appColor[1]);
      drawBattery();
      updateTimer = millis();
    }
    if (ts.touched()) {
      tPoint = ts.getPoint();
      x = map(tPoint.x, 0, 240, 240, 0);
      y = map(tPoint.y, 0, 320, 320, 0);
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
      } else if (x >= 180 && y >= 50 && x <= 240 && y <= 90 && !noFONA) {
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
          if (!noFONA) {
            fona.sendSMS(givenPNumber, message);
          }
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
            tft.drawRect(10, 95, 220, 40, black);
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
      } else if (page == SMS_READ && !noFONA) {
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
            tft.setTextColor(black, white);
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
  for (int i = 0; i < 20; i++) {
    message[i] = ' ';
  }
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
    Serial.println("SMS loading...");
    fona.readSMS(i, tempBuff1, 21, &lengthBuff);
    Serial.println("Loaded!");
    if (lengthBuff > 0) {
      Serial.println("SMS not null, copying...");
      fona.getSMSSender(i, tempBuff2, 21);
      memcpy(message3, message2, 21);
      memcpy(message2, message1, 21);
      memcpy(message1, tempBuff1, 21);
      memcpy(sender3, sender2, 21);
      memcpy(sender2, sender1, 21);
      memcpy(sender1, tempBuff2, 21);
      Serial.println("Done");
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

void setApp() {
  draw(APP_SETTINGS);
  int oldX = 0;
  while (appExit == false) {
    if (digitalRead(BTN_HOME) == LOW) {
      appExit = true;
      while (digitalRead(BTN_HOME) == LOW) {}
    }
    if (millis() - updateTimer >= 20000) {
      drawTime(appColor[2]);
      drawBattery();
      updateTimer = millis();
    }
    if (ts.touched()) {
      tPoint = ts.getPoint();
      x = map(tPoint.x, 0, 240, 240, 0);
      y = map(tPoint.y, 0, 320, 320, 0);
      if (x >= 20 && y >= 170 && x <= 220 && y <= 210) {
        while (ts.touched()) {
          tPoint = ts.getPoint();
          x = map(tPoint.x, 0, 240, 240, 0);
          if (x >= 20 && x <= 220) {
            if (oldX != x) {
              oldX = x;
              tft.fillRect(map(volume, 1, 10, 20, 210), 170, 10, 40, white);
              tft.drawFastHLine(20, 190, 200, black);
              tft.drawFastVLine(20, 170, 40, black);
              tft.drawFastVLine(220, 170, 40, black);
              tft.drawFastVLine(120, 175, 30, black);
              tft.drawFastVLine(70, 180, 20, black);
              tft.drawFastVLine(170, 180, 20, black);
              volume = map(x, 20, 220, 1, 10);
              tft.fillRect(map(volume, 1, 10, 20, 210), 170, 10, 40, darkgrey);
            }
          }
        }
        if (!noFONA) {
          fona.setAllVolumes(volume * 10);
          fona.playToolkitTone(6, 1000);
          if (!noSD) {
            SD.remove("SET_VOL.txt");
            SDfile = SD.open("SET_VOL.txt", FILE_WRITE);
            SDfile.print(volume);
            Serial.print("Setting saved on SD: Volume = ");
            Serial.println(volume);
            SDfile.close();
          }
        }
      } else if (x >= 20 && y >= 90 && x <= 220 && y <= 130) {
        while (ts.touched()) {
          tPoint = ts.getPoint();
          x = map(tPoint.x, 0, 240, 240, 0);
          if (x >= 20 && x <= 220) {
            if (oldX != x) {
              oldX = x;
              tft.fillRect(map(bl, 1, 10, 20, 210), 90, 10, 40, white);
              tft.drawFastHLine(20, 110, 200, black);
              tft.drawFastVLine(20, 90, 40, black);
              tft.drawFastVLine(220, 90, 40, black);
              tft.drawFastVLine(120, 95, 30, black);
              tft.drawFastVLine(70, 100, 20, black);
              tft.drawFastVLine(170, 100, 20, black);
              bl = map(x, 20, 220, 1, 10);
              tft.fillRect(map(bl, 1, 10, 20, 210), 90, 10, 40, darkgrey);
            }
          }
          backlight(bl);
        }
        if (!noSD) {
          SD.remove("SET_BL.txt");
          SDfile = SD.open("SET_BL.txt", FILE_WRITE);
          SDfile.print(bl);
          Serial.print("Setting saved on SD: Backlight level = ");
          Serial.println(bl);
          SDfile.close();
        }
      } else if (x >= 30 && y >= 260 && x <= 120 && y <= 310) {
        audio = FONA_EXTAUDIO;
        fona.setAudio(audio);
        fona.playToolkitTone(6, 1000);
        if (!noSD) {
          SD.remove("AUD_TYP.txt");
          SDfile = SD.open("AUD_TYP.txt", FILE_WRITE);
          SDfile.print(audio);
          Serial.print("Setting saved on SD: Audio out = ");
          Serial.println(audio);
          SDfile.close();
        }
      } else if (x >= 120 && y >= 260 && x <= 210 && y <= 310) {
        audio = FONA_HEADSETAUDIO;
        fona.setAudio(audio);
        fona.playToolkitTone(6, 1000);
        if (!noSD) {
          SD.remove("AUD_TYP.txt");
          SDfile = SD.open("AUD_TYP.txt", FILE_WRITE);
          SDfile.print(audio);
          Serial.print("Setting saved on SD: Audio out = ");
          Serial.println(audio);
          SDfile.close();
        }
      }
      while (ts.touched()) {}
    }
  }
  appExit = false;
  exitApp();
}




void pongApp() {
  draw(APP_PONG);
  byte ballX = 120;
  byte ballY = 160;
  byte ballXSpeed;
  byte ballYSpeed;
  byte paddleX = 80;
  byte oldPaddleX = paddleX;
  bool gameOn = false;
  while (appExit == false) {
    if (digitalRead(BTN_HOME) == LOW) {
      appExit = true;
      while (digitalRead(BTN_HOME) == LOW) {}
    }
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

      tPoint = ts.getPoint();
      tPoint.x = map(tPoint.x, 0, 240, 240, 0);
      tPoint.y = map(tPoint.y, 0, 320, 320, 0);
      if (gameOn == false && tPoint.x >= 80 && tPoint.y >= 100 && tPoint.x <= 160 && tPoint.y <= 160) {
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
        if (tPoint.x <= 40) {
          paddleX = 0;
        } else {
          paddleX = (tPoint.x - 40);
        }
        if (paddleX > 160) {
          paddleX = 160;
        }
      }
    }
  }
  appExit = false;
  exitApp();
}

void exitApp() {
  int i;
  for (i = 50; i <= 320; i++) {
    tft.drawFastHLine(0, i, 240, cyan);
    tft.drawFastVLine(map(i, 50, 320, 0, 240), 0, 50, blue);
  }
  draw(MENU);
  appOnScreen = MENU;
  lockCount = 0;
  lockTimer = millis();
}

void lock() {
  if (!debug) {
    for (int i = bl; i >= 0; i--) {
      backlight(i);
      delay(40);
    }
  } else {
    tft.setCursor(50, 50);
    tft.setTextSize(5);
    tft.setTextColor(black, white);
  }
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
                  while (getTouchPart() == 2 && ts.touched()) {}
                  if (!ts.touched()) {
                    unlocked = true;
                  }
                }
              }
            }
          }
        }
      }
    }
    if (debug) {
      tft.print(getTouchPart());
      tft.setCursor(50, 50);
    }
  }
  for (int i = 0; i <= bl; i++) {
    backlight(i);
    delay(20);
  }
  while (ts.touched()) {}
}

int getTouchPart() {
  tPoint = ts.getPoint();
  x = map(tPoint.x, 0, 240, 240, 0);
  y = map(tPoint.y, 0, 320, 320, 0);
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
    tPoint = ts.getPoint();
    x = map(tPoint.x, 0, 240, 240, 0);
    y = map(tPoint.y, 0, 320, 320, 0);
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

void radioApp() {
  draw(APP_RADIO);
  while (!appExit) {
    if (digitalRead(BTN_HOME) == LOW) {
      appExit = true;
      while (digitalRead(BTN_HOME) == LOW) {}
    }
    if (millis() - updateTimer >= 20000) {
      drawTime(appColor[4]);
      drawBattery();
      updateTimer = millis();
    }
    if (ts.touched()) {
      tPoint = ts.getPoint();
      x = map(tPoint.x, 0, 240, 240, 0);
      y = map(tPoint.y, 0, 320, 320, 0);
      tft.setTextSize(4);
      tft.setTextColor(black, white);
      if (x >= 20 && y >= 60 && x <= 60 && y <= 90) {
        channelNums[0]++;
        if (channelNums[0] > 9) {
          channelNums[0] = 0;
        }
        tft.setCursor(30, 100);
        tft.print(channelNums[0]);
      } else if (x >= 70 && y >= 60 && x <= 110 && y <= 90) {
        channelNums[1]++;
        if (channelNums[1] > 9) {
          channelNums[1] = 0;
        }
        tft.setCursor(80, 100);
        tft.print(channelNums[1]);
      } else if (x >= 120 && y >= 60 && x <= 160 && y <= 90) {
        channelNums[2]++;
        if (channelNums[2] > 9) {
          channelNums[2] = 0;
        }
        tft.setCursor(130, 100);
        tft.print(channelNums[2]);
      } else if (x >= 180 && y >= 60 && x <= 220 && y <= 90) {
        channelNums[3]++;
        if (channelNums[3] > 9) {
          channelNums[3] = 0;
        }
        tft.setCursor(190, 100);
        tft.print(channelNums[3]);
      } else if (x >= 20 && y >= 140 && x <= 60 && y <= 170) {
        if (channelNums[0] == 0) {
          channelNums[0] = 9;
        } else {
          channelNums[0]--;
        }
        tft.setCursor(30, 100);
        tft.print(channelNums[0]);
      } else if (x >= 70 && y >= 140 && x <= 110 && y <= 170) {
        if (channelNums[1] == 0) {
          channelNums[1] = 9;
        } else {
          channelNums[1]--;
        }
        tft.setCursor(80, 100);
        tft.print(channelNums[1]);
      } else if (x >= 120 && y >= 140 && x <= 160 && y <= 170) {
        if (channelNums[2] == 0) {
          channelNums[2] = 9;
        } else {
          channelNums[2]--;
        }
        tft.setCursor(130, 100);
        tft.print(channelNums[2]);
      } else if (x >= 180 && y >= 140 && x <= 220 && y <= 170) {
        if (channelNums[3] == 0) {
          channelNums[3] = 9;
        } else {
          channelNums[3]--;
        }
        tft.setCursor(190, 100);
        tft.print(channelNums[3]);
      } else if (x >= 30 && y >= 190 && x <= 210 && y <= 230) {
        drawText("TUNE", 85, 200, 3, black, green);
        while (ts.touched()) {}
        drawText("TUNE", 85, 200, 3, white, green);
        if (!noFONA) {
          fona.FMradio(true, audio);
          channelNum = ((channelNums[0] * 1000) + (channelNums[1] * 100) + (channelNums[2] * 10) + (channelNums[3]));
          fona.tuneFMradio(channelNum);
        }
      } else if (x >= 30 && y >= 240 && x <= 210 && y <= 280) {
        drawText("CLOSE", 75, 250, 3, black, darkgrey);
        while (ts.touched()) {}
        drawText("CLOSE", 75, 250, 3, white, darkgrey);
        if (!noFONA) {
          fona.FMradio(false);
        }
      }
      while (ts.touched()) {}
    }
  }
  appExit = false;
  exitApp();
}

void clockApp() {
  draw(APP_CLOCK);
  int xMinute = 120;
  int yMinute = 80;
  int xHour = 120;
  int yHour = 80;
  int xSec = 120;
  int ySec = 80;
  int seconds = 0;
  int minutes = 0;
  int hours = 0;
  if (!noFONA) {
    fona.getTime(RTCtime, 23);
    seconds = ((RTCtime[16] - '0') * 10) + (RTCtime[17] - '0');
    minutes = ((RTCtime[13] - '0') * 10) + (RTCtime[14] - '0');
    hours = ((RTCtime[10] - '0') * 10) + (RTCtime[11] - '0');
  }
  float mAngle = 0;
  float hAngle = 0;
  float sAngle = 0;
  long int timer = 0;
  const int cenX = 120;
  const int cenY = 160;
  while (!appExit) {
    if (digitalRead(BTN_HOME) == LOW) {
      appExit = true;
      while (digitalRead(BTN_HOME) == LOW) {}
    }
    if (ts.touched()) {
      tPoint = ts.getPoint();
      x = map(tPoint.x, 0, 240, 240, 0);
      y = map(tPoint.y, 0, 320, 320, 0);
    }
    if (millis() - updateTimer >= 20000) {
      drawTime(appColor[5]);
      drawBattery();
      updateTimer = millis();
    }
    if (millis() - timer >= 1000) {
      timer = millis();

      tft.drawLine(cenX, cenY, xHour, yHour, white);
      tft.drawLine(cenX, cenY, xMinute, yMinute, white);
      tft.drawLine(cenX, cenY, xSec, ySec, white);

      mAngle = (PI * 2) / 60 * minutes;
      xMinute = cenX - (100 * sin(mAngle));
      yMinute = cenY + (100 * cos(mAngle));
      yMinute = map(yMinute, 60, 260, 260, 60);
      xMinute = map(xMinute, 20, 220, 220, 20);

      hAngle = (PI * 2) / 12 * hours;
      xHour = cenX - (70 * sin(hAngle));
      yHour = cenY + (70 * cos(hAngle));
      yHour = map(yHour, 60, 260, 260, 60);
      xHour = map(xHour, 20, 220, 220, 20);

      sAngle = (PI * 2) / 60 * seconds;
      xSec = cenX - (90 * sin(sAngle));
      ySec = cenY + (90 * cos(sAngle));
      ySec = map(ySec, 60, 260, 260, 60);
      xSec = map(xSec, 20, 220, 220, 20);

      tft.drawLine(cenX, cenY, xHour, yHour, white);
      tft.drawLine(cenX, cenY, xMinute, yMinute, white);
      tft.drawLine(cenX, cenY, xSec, ySec, white);

      tft.drawLine(cenX, cenY, xMinute, yMinute, red);
      tft.drawLine(cenX, cenY, xSec, ySec, blue);
      tft.drawLine(cenX, cenY, xHour, yHour, black);
      tft.fillCircle(cenX, cenY, 3, black);

      tft.setTextColor(black, white);
      tft.setTextSize(3);
      tft.setCursor(50, 280);
      if (hours < 10) {
        tft.print("0");
      }
      tft.print(hours);
      tft.print(":");
      if (minutes < 10) {
        tft.print("0");
      }
      tft.print(minutes);
      tft.print(":");
      if (seconds < 10) {
        tft.print("0");
      }
      tft.print(seconds);

      seconds++;
      if (seconds > 59) {
        seconds = 0;
        minutes++;
        updateTimer = 0;
      }
      if (minutes > 59) {
        minutes = 0;
        hours++;
      }
      if (hours > 23) {
        hours = 0;
      }
    }
  }
  appExit = false;
  exitApp();
}

void paintApp() {
  draw(APP_PAINT);
  uint16_t selColor = red;
  int brushWidth = 3;
  int oldX = 0;
  int oldY = 50;
  while (!appExit) {
    if (digitalRead(BTN_HOME) == LOW) {
      appExit = true;
      while (digitalRead(BTN_HOME) == LOW) {}
    }
    if (millis() - updateTimer >= 20000) {
      drawTime(appColor[6]);
      drawBattery();
      updateTimer = millis();
    }
    if (ts.touched()) {
      tPoint = ts.getPoint();
      x = map(tPoint.x, 0, 240, 240, 0);
      y = map(tPoint.y, 0, 320, 320, 0);
      if (y >= 50 && y <= (280 - brushWidth)) {
        tft.drawLine(oldX, oldY, x, y, selColor);
        oldX = x;
        oldY = y;
      }
      if (y >= 280) {
        if (x <= 43 && y <= 315) {
          selColor = red;
          brushWidth = 3;
        } else if (x >= 43 && x <= 81 && y <= 315) {
          selColor = green;
          brushWidth = 3;
        } else if (x >= 81 && x <= 119 && y <= 315) {
          selColor = blue;
          brushWidth = 3;
        } else if (x >= 119 && x <= 157 && y <= 315) {
          selColor = black;
          brushWidth = 3;
        } else if (x >= 157 && x <= 195 && y <= 315) {
          selColor = white;
          brushWidth = 5;
        } else if (x >= 195 && y <= 315) {
          tft.fillRect(0, 50, 240, 230, white);
        }
      }
    }
  }
  appExit = false;
  exitApp();
}

void calcApp() {
  draw(APP_CALC);
  long int calcResult = 0;
  int calcNs[10] = {0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
  char calcOs[10] = {' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
  int calcIndex = 0;
  Serial.print(F("calcResult: "));
  Serial.print(calcResult);
  Serial.print(F(", calcNS[0]: "));
  Serial.print(calcNs[0]);
  Serial.print(F(", calcOs[0]: "));
  Serial.print(calcOs[0]);
  Serial.print(F(", calcIndex: "));
  Serial.println(calcIndex);
  while (!appExit) {
    if (digitalRead(BTN_HOME) == LOW) {
      appExit = true;
      while (digitalRead(BTN_HOME) == LOW) {}
    }
    if (millis() - updateTimer >= 20000) {
      drawTime(appColor[7]);
      drawBattery();
      updateTimer = millis();
    }
    if (ts.touched()) {
      tPoint = ts.getPoint();
      x = map(tPoint.x, 0, 240, 240, 0);
      y = map(tPoint.y, 0, 320, 320, 0);
      if (x >= 20 && y >= 110 && x <= 200 && y <= 300) { // If keypad pressed
        if (y <= 155) { // First row

          if (x <= 70) { // First column
            calcNs[calcIndex] *= 10;
            calcNs[calcIndex] += 1;
          } else if (x >= 70 && x <= 120) { // Second column
            calcNs[calcIndex] *= 10;
            calcNs[calcIndex] += 2;
          } else if (x >= 120 && x <= 170) { // Third column
            calcNs[calcIndex] *= 10;
            calcNs[calcIndex] += 3;
          } else if (x >= 170) { // Fouth column
            calcNs[calcIndex] *= 10;
            calcNs[calcIndex] += 4;
          }

        } else if (y >= 155 && y <= 205) { // Second row

          if (x <= 70) { // First column
            calcNs[calcIndex] *= 10;
            calcNs[calcIndex] += 5;
          } else if (x >= 70 && x <= 120) { // Second column
            calcNs[calcIndex] *= 10;
            calcNs[calcIndex] += 6;
          } else if (x >= 120 && x <= 170) { // Third column
            calcNs[calcIndex] *= 10;
            calcNs[calcIndex] += 7;
          } else if (x >= 170) { // Fouth column
            calcNs[calcIndex] *= 10;
            calcNs[calcIndex] += 8;
          }

        }
      }
      if (y >= 205 && y <= 255) { // Third row

        if (x <= 70) { // First column
          calcNs[calcIndex] *= 10;
          calcNs[calcIndex] += 9;
        } else if (x >= 70 && x <= 120) { // Second column
          calcNs[calcIndex] *= 10;
        } else if (x >= 120 && x <= 170) { // Third column
          if (calcNs[calcIndex] != 0) {
            Serial.println(F("Current num reset"));
            calcNs[calcIndex] /= 10;
          } else {
            Serial.println(F("All reset"));
            calcNs[0] = 0;
            calcOs[0] = 0;
            calcIndex = 0;
            calcResult = 0;
            for (int i = 1; i < 10; i++) {
              calcNs[i] = NULL;
              calcOs[i] = '\0';
            }
          }
        } else if (x >= 170) { // Fouth column
          calcResult = calculate(calcNs, calcOs);
          tft.setTextColor(black, white);
          tft.setTextSize(2);
          tft.setCursor(30, 70);
          int i = 0;
          while (calcNs[i] != 0) {
            tft.print(calcNs[i]);
            tft.print(calcOs[i]);
            i++;
          }
          tft.print("=");
          tft.print(calcResult);
          tft.print("              ");
          tft.drawRect(20, 60, 200, 40, black);
          delay(2000);
        }

      } else if (y >= 255) { // Fourth row

        if (x <= 70) { // First column
          calcOs[calcIndex] = '+';
          calcIndex++;
        } else if (x >= 70 && x <= 120) { // Second column
          calcOs[calcIndex] = '-';
          calcIndex++;
        } else if (x >= 120 && x <= 170) { // Third column
          calcOs[calcIndex] = '*';
          calcIndex++;
        } else if (x >= 170) { // Fouth column
          calcOs[calcIndex] = '/';
          calcIndex++;
        }

      }

      tft.setTextColor(black, white);
      tft.setTextSize(2);
      tft.setCursor(30, 70);
      tft.print(calcNs[calcIndex]);
      tft.print("              ");
      tft.drawRect(20, 60, 200, 40, black);
    }
    while (ts.touched()) {}
  }
  for (int i = 0; i < 10; i++) {
    calcNs[i] = 0;
    calcOs[i] = ' ';
  }
  appExit = false;
  exitApp();
}

int lengthof(char input[]) {
  for (int i = 0; i < 10; i++) {
    if (input[i] == '\0') {
      return i;
    }
  }
}

int calculate(int input1[], char input2[]) {
  int result = 0;
  Serial.print(F("Calculating with "));
  for (int i = 0; i < lengthof(input2) + 1; i++) {
    Serial.print(input1[i]);
    Serial.print(F(", "));
  }
  int opesLeft = lengthof(input2);
  Serial.print(F(" as numbers and "));
  Serial.print(input2);
  Serial.print(F(" (total: "));
  Serial.print(opesLeft);
  Serial.println(F(") as operators... "));
  int cNumBuff[10] = {input1[0]};
  for (int i = 0; i < opesLeft; i++) {
    switch (input2[i]) {
      case '+':
        if (i != 0) {
          cNumBuff[i] = cNumBuff[i - 1] + input1[i + 1];
        } else {
          cNumBuff[0] = input1[0] + input1[1];
        }
        Serial.println("Add");
        break;
      case '-':
        if (i != 0) {
          cNumBuff[i] = cNumBuff[i - 1] - input1[i + 1];
        } else {
          cNumBuff[0] = input1[0] - input1[1];
        }
        Serial.println("Substract");
        break;
      case '*':
        if (i != 0) {
          cNumBuff[i] = cNumBuff[i - 1] * input1[i + 1];
        } else {
          cNumBuff[0] = input1[0] * input1[1];
        }
        Serial.println("Multiply");
        break;
      case '/':
        if (i != 0) {
          cNumBuff[i] = cNumBuff[i - 1] / input1[i + 1];
        } else {
          cNumBuff[0] = input1[0] / input1[1];
        }
        Serial.println("Divide");
        break;
    }
  }
  for (int i = 0; i < 10; i++) {
    if (cNumBuff[i] != 0) {
      result = cNumBuff[i];
    }
  }
  Serial.print(F("Result was "));
  Serial.println(result);
  return result;
}

void contApp() {
  draw(APP_CONTACTS);
  char contName[5];
  char contNum[5];
  while (!appExit) {
    if (digitalRead(BTN_HOME) == LOW) {
      appExit = true;
      while (digitalRead(BTN_HOME) == LOW) {}
    }
    if (millis() - updateTimer >= 20000) {
      drawTime(appColor[8]);
      drawBattery();
      updateTimer = millis();
    }
    if (ts.touched()) {
      tPoint = ts.getPoint();
      x = map(tPoint.x, 0, 240, 240, 0);
      y = map(tPoint.y, 0, 320, 320, 0);
    }
    if (!noSD) {
      SDfile = SD.open("CON1NA");
      int i = 0;
      while (SDfile.available()) {
        contName[i] = SDfile.read();
        i++;
      }
      SDfile.close();

      SDfile = SD.open("CON1NU");
      i = 0;
      while (SDfile.available()) {
        contNum[i] = SDfile.read();
        i++;
      }
      SDfile.close();

      Serial.print(F("Contact name: '"));
      Serial.print(contName);
      Serial.print(F("', contact number: "));
      Serial.println(contNum);

      tft.setCursor(0, 70);
      tft.setTextSize(2);
      tft.setTextColor(black, white);
      tft.println(F("Contact name: '"));
      tft.println(contName);
      tft.println(F("', contact number: "));
      tft.println(contNum);
      while (1);
    }
  }
  appExit = false;
  exitApp();
}

void demo() {
  int oldDemo = 4;
  int demoNumber = 0;
  long nextTimer = 0;
  Serial.println(F("Play demo? [y/n]"));
  while (!Serial.available() && !ts.touched()) {}
  if (!ts.touched() && Serial.read() == 'y') {
    Serial.println(F("Demo playing..."));
    while (!ts.touched()) {
      for (int i = bl; i >= 0; i--) {
        backlight(i);
        delay(15);
      }
      do {
        demoNumber = random(0, 3);
      } while (demoNumber == oldDemo);
      oldDemo = demoNumber;
      switch (demoNumber) {
        case 0:
          tft.setTextSize(3);
          tft.setTextColor(white);
          tft.setCursor(0, 10);
          tft.fillScreen(navy);
          tft.fillRoundRect(10, 100, 220, 135, 10, cyan);
          tft.setTextSize(12);
          tft.setCursor(50, 120);
          tft.setTextColor(black);
          tft.print('F');
          tft.setTextSize(5);
          tft.print(F("at"));
          tft.setCursor(108, 168);
          tft.print(F("ONE"));
          for (int i = 0; i < bl; i++) {
            backlight(i);
            delay(15);
          }
          nextTimer = millis();
          while (millis() - nextTimer <= 5000 && !ts.touched());
          break;
        case 1:
          tft.fillScreen(navy);
          drawText("Hello there!", 20, 40, 3, white, navy);
          drawText("This is FatFone,", 10, 80, 2, white, navy);
          drawText("an open source", 10, 120, 2, white, navy);
          drawText("modular DIY", 20, 160, 3, white, navy);
          drawText("mobile phone.", 10, 200, 2, white, navy);
          for (int i = 0; i < bl; i++) {
            backlight(i);
            delay(15);
          }
          nextTimer = millis();
          while (millis() - nextTimer <= 5000 && !ts.touched());
          break;
        case 2:
          int demoX = 0;
          int demoY = 0;
          bool ballDirX = 1;
          bool ballDirY = 1;
          tft.fillScreen(random(0x0000, 0xFFFF));
          for (int i = 0; i < bl; i++) {
            backlight(i);
            delay(15);
          }
          nextTimer = millis();
          while (millis() - nextTimer <= 10000 && !ts.touched()) {
            tft.fillRect(demoX, demoY, 10, 10, random(0x0000, 0xFFFF));
            //delay(1);
            tft.fillRect(demoX, demoY, 10, 10, random(0x0000, 0xFFFF));

            if (demoX <= -5) {
              ballDirX = 1;
            } else if (demoX >= 235) {
              ballDirX = 0;
            }
            if (demoY <= -5) {
              ballDirY = 1;
            } else if (demoY >= 315) {
              ballDirY = 0;
            }

            if (ballDirX) {
              demoX += 4;
            } else {
              demoX -= 4;
            }
            if (ballDirY) {
              demoY += 4;
            } else {
              demoY -= 4;
            }
          }
          break;
      }
    }
  }
  exitApp();
}






















