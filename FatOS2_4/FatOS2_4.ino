// Sketch needs an AVR mC with >60KB of flash!

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
#define FONA_RST 4
#define FONA_RI 3
#define FONA_KEY A0
#define FONA_PWR A1
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

#define DEBUG(s) if (debug) Serial.print(s);
#define DEBUGLN(s) if (debug) Serial.println(s);

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

#define MENU        0
#define PHONE_MAIN  1
#define PHONE_KEYS  2
#define SMS_SEND    3
#define SMS_READ    4
#define SETTINGS    5
#define PONG        6
#define RADIO       7
#define CLOCK       8
#define PAINT       9
#define CALC        10
#define CONTACTS    11

uint16_t barcol[12] = {blue, red, red, green, green, lightgrey, black, navy, darkgrey, maroon, orange, navy};

#define KEYPAD_CHARS  -2
#define KEYPAD_NUMS   -1

#define CALC_PLUS 0
#define CALC_MINUS 1
#define CALC_MULT 2
#define CALC_DIV 3

bool debug = 0;

bool locked = 0;

bool incCall = 1;

bool noFONA = false;
bool noSD = false;

#define appSize 60
byte appLocX[9] = {10, 90, 170, 10, 90, 170, 10, 90, 170};
byte appLocY[9] = {60, 60, 60, 140, 140, 140, 220, 220, 220};
uint16_t appColor[9] = {red, green, lightgrey, black, navy, darkgrey, maroon, orange, navy};
char *appName[9] = {"Phone", "Messages", "Settings", "Pong", "Radio", "Clock", "Paint", "Calc", "Contacts"};
byte appNamePlusX[9] = {15, 7, 7, 17, 15, 15, 15, 17, 7};
byte appAmount = 9;
int prev[12] = {0, MENU, PHONE_MAIN, MENU, MENU, MENU, MENU, MENU, MENU, MENU, MENU, MENU};
#define appAnimFrames 20

int getAppVar(int a) {
  switch (a) {
    case MENU:
      return -1;
    case PHONE_MAIN:
    case PHONE_KEYS:
      return 0;
    case SMS_SEND:
    case SMS_READ:
      return 1;
    default:
      return a - 3;
  }
}

char errorFONA[5] = "!FONA";

uint8_t bl = 128;
bool audio = FONA_EXTAUDIO;
uint8_t volume = 255;

char RTCtime[23];

long updateTimer = 20000;

char numberBuf[13] = "             ";
int mesLocY[] = {100, 160, 220, 280};
int pNumCount = 0;

#define ACT_CALL 1
#define ACT_SMS 2
#define ACT_OTHER 3

char keypad[12] = {
  '1', '2', '3',
  '4', '5', '6',
  '7', '8', '9',
  '+', '0', '#'
};

char *charpad1[12] = {
  "abc", "def", "ghi",
  "jkl", "mno", "pqr",
  "stu", "vwx", "yz.",
  ",?!", "'\"-", "UPP"
};

char *charpad2[12] = {
  "ABC", "DEF", "GHI",
  "JKL", "MNO", "PQR",
  "STU", "VWX", "YZ.",
  ",?!", "'\"-", "LOW"
};

char chars[] = "aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ.!? ";
char message[21] = {' '};
int charOnScreenNum = 0;
int messageIndex = 0;

uint16_t channelNums[5] = {0, 0, 0, 0};
uint16_t channelNum = 0;

int x = 0;
int y = 0;
TS_Point tPoint;

File SDfile;

bool callFlag = 0;

long netTimeout = 0, lockTimer = 0;
int lockCount = 0, lockCycles = 12, currentActive = -1, netState = 0;

void drawButton(int x, int y, int w, int h, uint16_t c, char *text, int size, bool held, bool f);
void start(int a, bool animate);

struct Button {
  int x, y, w, h, s;
  uint16_t c = NULL;
  char *text;
  bool p = 0;
  void (*func)();
  int startAction = NULL;
  void init(int ix, int iy, int iw, int ih, uint16_t ic, char *itext, int is) {
    x = ix;
    y = iy;
    w = iw;
    h = ih;
    c = ic;
    text = itext;
    s = is;
    drawButton(x, y, w, h, c, text, s, 0, 1);
  }
  bool update() {
    tPoint = ts.getPoint();
    int tx = 240 - tPoint.x;
    int ty = 320 - tPoint.y;
    if (ts.touched() && tx >= x && tx <= x + w && ty >= y && ty <= y + h) {
      if (p == 0)
        drawButton(x, y, w, h, c, text, s, 1, 0);
      p = 1;
    } else {
      if (p == 1) {
        drawButton(x, y, w, h, c, text, s, 0, 0);
        if (!startAction) {
          (*func)();
        } else {
          start(startAction, 0);
        }
      }
      p = 0;
    }
    return p;
  }
};

Button btnnull;
Button btns[10] = {btnnull};

struct TextField {
  int x, y, w, h, s, l, pt = 0;
  bool active = 0, enablec = 1, changed = 1, reset = 0;
  char *c = malloc(1);
  void init(int ix, int iy, int iw, int ih, char *ic, int il, int is) {
    x = ix;
    y = iy;
    w = iw;
    h = ih;
    s = is;
    l = il + 1;
    c = realloc(c, l);
    strncpy(c, ic, l);
  }
  void enableChars(bool e) {
    enablec = e;
  }
  void append(char a) {
    changed = 1;
    int r = 0;
    while (c[r]) r++;
    if (((r < l - 1 || a == '\b') && r > 0) && a != '\0') tft.fillRect(x + 3, y + 3, w - 6, h - 6, white);
    if (a == '\b') {
      if (r > 0) {
        pt--;
        c[pt] = 0;
        update();
      }
    } else if (r < l - 1 && a != '\0') {
      c[pt] = a;
      pt++;
      update();
    }
  }
  void draw() {
    tft.fillRect(x, y, w, h, white);
    tft.drawRect(x, y, w, h, black);
    tft.drawRect(x + 1, y + 1, w - 2, h - 2, black);
    tft.drawRect(x + 2, y + 2, w - 4, h - 4, black);
    changed = 1;
    update();
  }
  bool update() {
    bool as = 0;
    if (ts.touched()) {
      tPoint = ts.getPoint();
      int tx = 240 - tPoint.x;
      int ty = 320 - tPoint.y;
      if (tx >= x && ty >= y && tx <= x + w && ty <= y + h) {
        active = 1;
        changed = 1;
        as = 1;
      }
    }
    if (!active && reset) {
      tft.fillRect(x + 3, y + 3, w - 6, h - 6, white);
      changed = 1;
      reset = 0;
    }
    if (changed) {
      int r = 0;
      while (c[r]) r++;
      int dx = (x + w / 2) - (r * s * 6 - s) / 2;
      int dy = (y + h / 2) - (s * 7) / 2;
      tft.setCursor(dx, dy);
      tft.setTextSize(s);
      if (r != 0) {
        tft.setTextColor(black, white);
        tft.print(c);
      } else {
        int dx = (x + w / 2) - (s * 6 - s) / 2;
        tft.setCursor(dx, dy);
      }
      if (r < l - 1 && active) {
        tft.setTextColor(blue);
        tft.print('|');
      }
    }
    return as;
  }
  char *getString() {
    return c;
  }
};

struct Keypad {
  int mode = 0; // 0 = nums, 1 = chars
  bool visible = 0;
  int cset = 0; // 0 = lower case, 1 = upper case
  TextField *tf1;
  TextField *tf2;
  int tfset = 0;
  char *ok = " OK ";
  int okaction = 0;
  void draw() {
    visible = 1;
    tft.fillRect(0, 170, 240, 30, blue);
    tft.setCursor(30, 178);
    tft.setTextSize(2);
    tft.setTextColor(white);
    tft.print("SPACE");
    tft.setCursor(150, 178);
    tft.print("ERASE");
    tft.fillRect(0, 200, 160, 120, lightgrey);
    tft.fillRect(160, 200, 80, 60, green);
    tft.fillRect(160, 260, 80, 60, darkgrey);
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 3; x++) {
        int dx = 0, dy = 0;
        if (!mode) {
          dx = x * 53 + 17;
          dy = y * 30 + 205;
          tft.setTextSize(3);
        } else {
          dx = x * 53 + 8;
          dy = y * 30 + 208;
          tft.setTextSize(2);
        }
        tft.setTextColor(black);
        tft.setCursor(dx, dy);
        if (!mode) {
          tft.print(keypad[x + y * 3]);
        } else {
          if (cset) {
            tft.print(charpad2[x + y * 3]);
          } else {
            tft.print(charpad1[x + y * 3]);
          }
        }
      }
    }
    tft.setTextColor(white);
    tft.setTextSize(3);
    int r = 0;
    while (ok[r]) r++;
    int dx = 200 - (r * 3 * 6) / 2;
    tft.setCursor(dx, 220);
    tft.print(ok);
    tft.setCursor(173, 280);
    if (!mode) {
      tft.print("ABC");
    } else {
      tft.print("123");
    }
  }
  void setOkText(char *t) {
    strncpy(ok, t, 4);
  }
  void attachTextField(TextField *setTf) {
    if (tfset == 0) {
      tf1 = setTf;
      tfset = 1;
    } else if (tfset == 1) {
      tf2 = setTf;
      tfset = 2;
    }
  }
  void detachTextFields() {
    tfset = 0;
  }
  void undraw() {
    visible = 0;
    detachTextFields();
  }
  void setMode(int smode) {
    mode = smode;
  }
  void setOkAction(int act) {
    okaction = act;
  }
  int kpressed = -1;
  char cpressed = 0;
  char update() {
    if (visible) {
      if (ts.touched()) {
        tPoint = ts.getPoint();
        int tx = 240 - tPoint.x;
        int ty = 320 - tPoint.y;
        int ktx = 0;
        int kty = 0;
        if (ty >= 200 && ty < 320) {
          if (tx < 160) {
            ktx = (tx) / 53;
            kty = (ty - 200) / 30;
            if (ktx > 2) ktx = 2;
            if (kty > 3) kty = 3;
            if (ktx < 0) ktx = 0;
            if (kty < 0) kty = 0;
            kpressed = ktx + kty * 3;
            if (mode) kpressed = -1;
            if (!mode) {
              tft.setTextSize(3);
              tft.setTextColor(white);
              int dx = ktx * 53 + 17;
              int dy = kty * 30 + 205;
              tft.setCursor(dx, dy);
              tft.print(keypad[ktx + kty * 3]);
            } else {
              tft.setTextSize(2);
              tft.setTextColor(white);
              int dx = ktx * 53 + 8;
              int dy = kty * 30 + 208;
              tft.setCursor(dx, dy);
              if (!cset) {
                tft.print(charpad1[ktx + kty * 3]);
              } else {
                tft.print(charpad2[ktx + kty * 3]);
              }
            }
            if (ktx == 2 && kty == 3 && mode) {
              tft.setTextSize(2);
              tft.setTextColor(white);
              tft.setCursor(114, 298);
              if (!cset) {
                tft.print(charpad1[11]);
              } else {
                tft.print(charpad2[11]);
              }
              cset = !cset;
              draw();
            }
            if (!(ktx == 2 && kty == 3) && mode) {
              tft.fillRect(0, 200, 160, 120, lightgrey);
              tft.setTextColor(black);
              tft.setTextSize(3);
              char *c;
              if (!cset) {
                c = charpad1[ktx + kty * 3];
              } else {
                c = charpad2[ktx + kty * 3];
              }
              tft.setCursor(31, 250);
              tft.print(c[0]);
              tft.setCursor(71, 250);
              tft.print(c[1]);
              tft.setCursor(111, 250);
              tft.print(c[2]);
              while (!ts.touched());
              tPoint = ts.getPoint();
              tx = 240 - tPoint.x;
              ty = 320 - tPoint.y;
              int cp = tx / 53;
              if (cp < 0) cp = 0;
              if (cp > 2) cp = 2;
              cpressed = c[cp];
              tft.setTextColor(white);
              switch (cp) {
                case 0:
                  tft.setCursor(31, 250);
                  break;
                case 1:
                  tft.setCursor(71, 250);
                  break;
                case 2:
                  tft.setCursor(111, 250);
                  break;
              }
              tft.print(c[cp]);
              kpressed = cpressed;
            }
            if (!mode) kpressed = ktx + kty * 3;
          } else {
            if (ty > 260) {
              tft.setTextColor(black);
              tft.setTextSize(3);
              tft.setCursor(173, 280);
              if (!mode) {
                tft.print("ABC");
              } else {
                tft.print("123");
              }
              mode = !mode;
              if (!mode) cset = 0;
              draw();
              kpressed = -1;
            } else {
              tft.setTextColor(black);
              tft.setTextSize(3);
              int r = 0;
              while (ok[r]) r++;
              int dx = 200 - (r * 3 * 6) / 2;
              tft.setCursor(dx, 220);
              tft.print(ok);
              kpressed = 12;
              switch (okaction) {
                case ACT_CALL:
                  fonaCall(tf1->getString());
                  break;
                case ACT_SMS:
                  fonaSMS(tf1->getString(), tf2->getString());
                  break;
              }
            }
          }
        } else if (ty >= 170 && ty < 320) {
          if (tx < 120) {
            tft.setCursor(30, 178);
            tft.setTextSize(2);
            tft.setTextColor(black);
            tft.print("SPACE");
            kpressed = 13;
          } else {
            tft.setTextSize(2);
            tft.setTextColor(black);
            tft.setCursor(150, 178);
            tft.print("ERASE");
            kpressed = 14;
          }
        }
        if (kpressed != -1) {
          int tmp = kpressed;
          if (!(tmp <= 12 || tmp == 14)){
            tmp = -1;
          }
          if (tf1->active && tfset > 0) {
            if (tf1->enablec) {
              tf1->append(convert(kpressed));
            } else {
              tf1->append(convert(tmp));
            }
          } else if (tf2->active && tfset > 1) {
            if (tf2->enablec) {
              tf2->append(convert(kpressed));
            } else {
              tf2->append(convert(tmp));
            }
          }
          while (ts.touched());
          return tmp;
        }
      }
      if (cpressed) {
        draw();
        cpressed = 0;
      } else if (kpressed != -1) {
        if (kpressed < 12) {
          int dx, dy;
          if (!mode) {
            tft.setTextSize(3);
            dx = (kpressed % 3) * 53 + 17;
            dy = (kpressed / 3) * 30 + 205;
          } else {
            tft.setTextSize(2);
            dx = (kpressed % 3) * 53 + 8;
            dy = (kpressed / 3) * 30 + 208;
          }
          tft.setTextColor(black);
          tft.setCursor(dx, dy);
          if (!mode) {
            tft.print(keypad[kpressed]);
          } else if (!cset) {
            tft.print(charpad1[kpressed]);
          } else {
            tft.print(charpad2[kpressed]);
          }
        } else if (kpressed == 12) {
          tft.setTextColor(white);
          tft.setTextSize(3);
          int r = 0;
          while (ok[r]) r++;
          int dx = 200 - (r * 3 * 6) / 2;
          tft.setCursor(dx, 220);
          tft.print(ok);
        } else if (kpressed == 13) {
          tft.setCursor(30, 178);
          tft.setTextSize(2);
          tft.setTextColor(white);
          tft.print("SPACE");
        } else if (kpressed == 14) {
          tft.setTextSize(2);
          tft.setTextColor(white);
          tft.setCursor(150, 178);
          tft.print("ERASE");
        }
        kpressed = -1;
      }
    }
    return -1;
  }
  char convert(int kpr) {
    if (kpr > 15) return kpr;
    switch (kpr) {
      case -1:
      case 12:
        return '\0';
      case 13:
        return ' ';
      case 14:
        return '\b';
      case 9:
        return '+';
      case 10:
        return '0';
      case 11:
        return '#';
      default:
        return kpr + 1 + '0';
    }
  }
};

Keypad kp;

TextField textfield1;
TextField textfield2;

void setup() {
  ts.begin(40);
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  pinMode(FONA_PWR, INPUT);
  pinMode(FONA_KEY, OUTPUT);
  digitalWrite(FONA_KEY, HIGH);
  digitalWrite(TFT_BL, LOW);
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
    DEBUGLN(F("Reading settings from SD..."));
    SDfile = SD.open("SETTING.TXT", FILE_READ);
    bl = SDfile.read();
    volume = SDfile.read();
    audio = SDfile.read();
    DEBUGLN(bl);
    DEBUGLN(volume);
    DEBUGLN(audio);
  } else {
    drawText("NO SD CARD", 65, 250, 2, red, navy);
    bl = 128;
    volume = 50;
    audio = 1;
  }

  for (int i = 0; i < bl; i++) {
    backlight(i);
    delay(10);
  }
  //if (btnb) {
  noFONA = true;
  drawText("FONA BYPASSED", 43, 250, 2, green, navy);
  /*} else {
    tft.setCursor(85, 200);
    tft.setTextColor(white, navy);
    tft.setTextSize(3);
    tft.print('.');
    if (!digitalRead(FONA_PWR)) {
      digitalWrite(FONA_KEY, LOW);
      delay(2100); // Delay to get FONA started
      digitalWrite(FONA_KEY, HIGH);
      while (!digitalRead(FONA_PWR));
    }
    DEBUG(F("Connecting to FONA with (RX, TX): "));
    DEBUG(FONA_RX);
    DEBUG(F(", "));
    DEBUGLN(FONA_TX);
    tft.print('.');
    DEBUGLN(F("Starting FONA 1/2..."));
    fonaSerial->begin(4800);
    tft.print('.');
    DEBUGLN(F("Starting FONA 2/2..."));
    if (!fona.begin(*fonaSerial)) {
      DEBUGLN(F("Couldn't start FONA"));
      drawText("FONA ERROR", 63, 250, 2, red, navy);
      delay(3000);
      asm volatile("jmp 0");
    }
    fona.hangUp();
    DEBUGLN(F("FONA started"));
    fona.setAudio(audio);
    fona.setPWM(2000);
    fona.setAllVolumes(volume);
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
    fona.callerIdNotification(1, digitalPinToInterrupt(3));
    delay(1000);
    }*/

  start(MENU, 1);
}

void loop() {
  updateAll();
  if (millis() - updateTimer >= 20000) {
    updateTimer = millis();
  }
  if (ts.touched()) {
    lockTimer = millis();
    lockCount = 0;
    if (currentActive == MENU) {
      menuTouchHandler();
    } else {
      if (ts.getPoint().y > 300) {
        if (prev[currentActive] == MENU) {
          start(prev[currentActive], 1);
        } else {
          start(prev[currentActive], 0);
        }
      }
    }
  }
  if (millis() - lockTimer >= 5000) {
    lockCount++;
    if (lockCount >= lockCycles) {
      lockCount = 0;
      lock();
      lockTimer = millis();
    }
    lockTimer = millis();
  }
}

void draw(int a) {
  if (a != PONG) {
    drawTime(a);
    drawBattery(a);
  }
  btns[0] = btnnull;
  kp.undraw();
  switch (a) {
    case MENU:
      {
        for (byte b = 0; b < appAmount; b++) {
          tft.fillRect(appLocX[b], appLocY[b], appSize, appSize, appColor[b]);
          drawText(appName[b], (appLocX[b] + appNamePlusX[b]), (appLocY[b] + 26), 1, white, appColor[b]);
        }
        Button p;
        p.init(5, 5, 40, 40, red, "", 3);
        tft.fillCircle(25, 26, 10, 0b1001100000000000);
        tft.fillCircle(25, 26, 7, red);
        for (int i = 22; i <= 28; i++) {
          if (i >= 24 && i <= 26) {
            tft.drawFastVLine(i, 13, 11, 0b1001100000000000);
          } else {
            tft.drawFastVLine(i, 13, 11, red);
          }
        }
        p.func = &lock;
        btns[0] = p;
        btns[1] = btnnull;
        updateTimer = millis();
      }
      break;
    case PHONE_MAIN:
      {
        tft.fillRect(0, 20, 240, 300, white);
        Button p;
        Button e;
        Button k;
        p.init(10, 30, 108, 60, green, "PICK UP", 2);
        e.init(122, 30, 108, 60, red, "HANG UP", 2);
        k.init(10, 100, 220, 80, lightgrey, "KEYPAD", 3);
        p.func = &fonaPickUp;
        e.func = &fonaHangUp;
        k.startAction = PHONE_KEYS;
        btns[0] = p;
        btns[1] = e;
        btns[2] = k;
        btns[3] = btnnull;
      }
      break;
    case PHONE_KEYS:
      {
        tft.fillRect(0, 20, 240, 300, white);
        textfield1.init(10, 40, 220, 50, "", 14, 2);
        textfield1.active = 1;
        textfield1.draw();
        kp.setOkText("CALL");
        textfield1.enableChars(0);
        kp.draw();
        kp.setOkAction(ACT_CALL);
        kp.attachTextField(&textfield1);
        btns[0] = btnnull;
      }
      break;
    case SMS_SEND:
      {
        tft.fillRect(0, 20, 240, 300, white);
        Button s;
        Button r;
        s.init(0, 25, 120, 30, lightgrey, "SEND", 2);
        r.init(120, 25, 120, 30, lightgrey, "READ", 2);
        s.startAction = SMS_SEND;
        r.startAction = SMS_READ;
        btns[0] = s;
        btns[1] = r;
        btns[2] = btnnull;
        textfield1.init(10, 60, 220, 40, "", 14, 2);
        textfield2.init(10, 105, 220, 40, "", 14, 2);
        textfield1.enableChars(0);
        textfield1.active = 1;
        textfield1.draw();
        textfield2.draw();
        kp.setOkText("SEND");
        kp.draw();
        kp.setOkAction(ACT_SMS);
        kp.attachTextField(&textfield1);
        kp.attachTextField(&textfield2);
      }
      break;
    case SETTINGS:
      tft.fillRect(0, 20, 240, 300, white);
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
      tft.setTextSize(1);
      tft.setTextColor(black, lightgrey);
      tft.setCursor(45, 285);
      tft.print(F("LOUDSPEAKER"));
      tft.setCursor(135, 285);
      tft.print(F("HEADPHONES"));
      break;
    case PONG:
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
    case RADIO:
      tft.fillRect(0, 20, 240, 300, white);
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
    case CLOCK:
      tft.fillRect(0, 20, 240, 300, white);
      tft.drawCircle(120, 160, 101, black);
      tft.drawCircle(120, 160, 102, black);
      break;
    case PAINT:
      tft.fillRect(0, 20, 240, 300, white);
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
    case CALC:
      tft.fillRect(0, 20, 240, 300, white);
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
    case CONTACTS:
      tft.fillRect(0, 20, 240, 300, white);
      break;
  }
}

void drawTime(char a) {
  if (noFONA) {
    for (int i = 10; i < 15; i++) {
      RTCtime[i] = errorFONA[i - 10];
    }
  } else {
    fona.getTime(RTCtime, 23);
  }
  if (a != MENU) {
    tft.setTextSize(2);
    //tft.setCursor(90, 3);
    tft.setCursor(5, 3);
  } else {
    tft.setTextSize(4);
    tft.setCursor(60, 11);
  }
  tft.setTextColor(white, barcol[a]);
  for (int i = 10; i < 15; i++) {
    tft.print(RTCtime[i]);
  }
}

void drawBattery(char a) {
  uint16_t batt = getBattery();
  int r = 0;
  int g = 0;
  if (batt > 30) {
    g = 63;
  } else if (batt > 20) {
    g = 63;
    r = 31;
  } else {
    r = 31;
  }
  uint16_t color = to565(r, g, 0);
  if (a != MENU) {
    tft.fillRect(185, 5, 50, 10, white);
    tft.drawRect(185, 5, 50, 10, black);
    tft.drawRect(184, 4, 52, 12, black);
    tft.fillRect(236 - batt / 2, 6, batt / 2 - 2, 8, color);
  } else {
    tft.fillRect(215, 5, 20, 40, white);
    tft.drawRect(215, 5, 20, 40, black);
    tft.drawRect(214, 4, 22, 42, black);
    tft.fillRect(216, 46 - batt / 2.5, 18, batt / 2.5 - 1, color);
  }
}

uint16_t getBattery() {
  uint16_t bat;
  if (noFONA) {
    bat = random(0, 100);
  } else {
    fona.getBattPercent(&bat);
  }
  return bat;
}

void start(int a, bool animate) {
  if (currentActive == a) return;
  currentActive = a;
  int anim = getAppVar(a);
  int sizeX, sizeY;
  if (animate) {
    if (a == MENU) {
      for (int i = 50; i <= 320; i++) {
        tft.drawFastHLine(0, i, 240, cyan);
        tft.drawFastVLine(map(i, 50, 320, 0, 240), 0, 50, blue);
      }
    } else {
      for (int i = 0; i <= appAnimFrames; i++) {
        x = map(i, 0, appAnimFrames, appLocX[anim], 0);
        y = map(i, 0, appAnimFrames, appLocY[anim], 0);
        sizeX = map(i, 0, appAnimFrames, appSize, 240);
        sizeY = map(i, 0, appAnimFrames, appSize, 20);
        tft.fillRect(x, y, sizeX, sizeY, appColor[anim]);
      }
    }
  }
  draw(a);
}

void drawText(char* text, byte locX, byte locY, byte textSize, uint16_t color, uint16_t bgcolor) {
  tft.setCursor(locX, locY);
  tft.setTextColor(color, bgcolor);
  tft.setTextSize(textSize);
  tft.print(text);
}

void backlight(uint8_t a) {
  analogWrite(TFT_BL, a);
}

void menuTouchHandler() {
  tPoint = ts.getPoint();
  x = 240 - tPoint.x;
  y = 320 - tPoint.y;
  if (y > 312) return;
  if (x >= appLocX[0] && y >= appLocY[0] && x <= (appLocX[0] + appSize) && y <= (appLocY[0] + appSize)) {
    drawText(appName[0], (appLocX[0] + appNamePlusX[0]), (appLocY[0] + 26), 1, white, appColor[0]);
    while (ts.touched()) {}
    drawText(appName[0], (appLocX[0] + appNamePlusX[0]), (appLocY[0] + 26), 1, black, appColor[0]);
    start(PHONE_MAIN, 1);
  } else if (x >= appLocX[1] && y >= appLocY[1] && x <= (appLocX[1] + appSize) && y <= (appLocY[1] + appSize)) {
    drawText(appName[1], (appLocX[1] + appNamePlusX[1]), (appLocY[1] + 26), 1, white, appColor[1]);
    while (ts.touched()) {}
    drawText(appName[1], (appLocX[1] + appNamePlusX[1]), (appLocY[1] + 26), 1, black, appColor[1]);
    start(SMS_SEND, 1);
  } else if (x >= appLocX[2] && y >= appLocY[2] && x <= (appLocX[2] + appSize) && y <= (appLocY[2] + appSize)) {
    drawText(appName[2], (appLocX[2] + appNamePlusX[2]), (appLocY[2] + 26), 1, white, appColor[2]);
    while (ts.touched()) {}
    drawText(appName[2], (appLocX[2] + appNamePlusX[2]), (appLocY[2] + 26), 1, black, appColor[2]);
    start(SETTINGS, 1);
  } else if (x >= appLocX[3] && y >= appLocY[3] && x <= (appLocX[3] + appSize) && y <= (appLocY[3] + appSize)) {
    drawText(appName[3], (appLocX[3] + appNamePlusX[3]), (appLocY[3] + 26), 1, white, appColor[3]);
    while (ts.touched()) {}
    drawText(appName[3], (appLocX[3] + appNamePlusX[3]), (appLocY[3] + 26), 1, black, appColor[3]);
    start(PONG, 1);
  } else if (x >= appLocX[4] && y >= appLocY[4] && x <= (appLocX[4] + appSize) && y <= (appLocY[4] + appSize)) {
    drawText(appName[4], (appLocX[4] + appNamePlusX[4]), (appLocY[4] + 26), 1, white, appColor[4]);
    while (ts.touched()) {}
    drawText(appName[4], (appLocX[4] + appNamePlusX[4]), (appLocY[4] + 26), 1, black, appColor[4]);
    start(RADIO, 1);
  } else if (x >= appLocX[5] && y >= appLocY[5] && x <= (appLocX[5] + appSize) && y <= (appLocY[5] + appSize)) {
    drawText(appName[5], (appLocX[5] + appNamePlusX[5]), (appLocY[5] + 26), 1, white, appColor[5]);
    while (ts.touched()) {}
    drawText(appName[5], (appLocX[5] + appNamePlusX[5]), (appLocY[5] + 26), 1, black, appColor[5]);
    start(CLOCK, 1);
  } else if (x >= appLocX[6] && y >= appLocY[6] && x <= (appLocX[6] + appSize) && y <= (appLocY[6] + appSize)) {
    drawText(appName[6], (appLocX[6] + appNamePlusX[6]), (appLocY[6] + 26), 1, white, appColor[6]);
    while (ts.touched()) {}
    drawText(appName[6], (appLocX[6] + appNamePlusX[6]), (appLocY[6] + 26), 1, black, appColor[6]);
    start(PAINT, 1);
  } else if (x >= appLocX[7] && y >= appLocY[7] && x <= (appLocX[7] + appSize) && y <= (appLocY[7] + appSize)) {
    drawText(appName[7], (appLocX[7] + appNamePlusX[7]), (appLocY[7] + 26), 1, white, appColor[7]);
    while (ts.touched()) {}
    drawText(appName[7], (appLocX[7] + appNamePlusX[7]), (appLocY[7] + 26), 1, black, appColor[7]);
    start(CALC, 1);
  } else if (x >= appLocX[8] && y >= appLocY[8] && x <= (appLocX[8] + appSize) && y <= (appLocY[8] + appSize)) {
    drawText(appName[8], (appLocX[8] + appNamePlusX[8]), (appLocY[8] + 26), 1, white, appColor[8]);
    while (ts.touched()) {}
    drawText(appName[8], (appLocX[8] + appNamePlusX[8]), (appLocY[8] + 26), 1, black, appColor[8]);
    start(CONTACTS, 1);
  } else {
    if (debug) {
      tft.invertDisplay(true);
      while (ts.touched()) {}
      tft.invertDisplay(false);
    }
  }
}

void updateSD() {
  SD.remove("SETTING.TXT");
  SDfile = SD.open("SETTING.TXT", FILE_WRITE);
  SDfile.print(bl);
  SDfile.print(volume);
  SDfile.print(audio);
  debug = 1;
  DEBUGLN("Settings saved on SD");
  debug = 0;
  SDfile.close();
}

void lock() {
  if (!debug) {
    for (int i = bl; i >= 0; i--) {
      backlight(i);
      delay(5);
    }
  } else {
    tft.setCursor(50, 50);
    tft.setTextSize(5);
    tft.setTextColor(black, white);
  }
  bool unlocked = false;
  while (!unlocked) {
    if (ts.touched() && getTouchPart() == 1) {
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
    if (debug) {
      tft.print(getTouchPart());
      tft.setCursor(50, 50);
    }
  }
  for (int i = 0; i <= bl; i++) {
    backlight(i);
    delay(1);
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

void drawButton(int x, int y, int w, int h, uint16_t c, char *text, int size, bool held, bool f) {
  int r, g, b;
  getRGB(c, r, g, b);
  uint16_t d = to565((float)r * 0.6, (float)g * 0.6, (float)b * 0.6);
  if (f) {
    tft.fillRect(x, y, w, h, d);
    tft.fillRect(x + 3, y + 3, w - 6, h - 6, c);
    tft.setTextColor(d);
    int tx, ty;
    getCenter(text, size, x + w / 2, y + h / 2, tx, ty);
    tft.setCursor(tx, ty);
    tft.setTextSize(size);
    tft.print(text);
  } else if (held) {
    for (int i = 0; i < 3; i++) {
      tft.drawRect(x + i, y + i, w - i * 2, h - i * 2, c);
    }
  } else {
    for (int i = 0; i < 3; i++) {
      tft.drawRect(x + i, y + i, w - i * 2, h - i * 2, d);
    }
  }
}

uint16_t to565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0b11111) << 11) | ((g & 0b111111) << 5) | (b & 0b11111);
}

void getRGB(uint16_t c, int &r, int &g, int &b) {
  r = c >> 11;
  g = (c >> 5) & 0b111111;
  b = c & 0b11111;
}

void getCenter(char *text, int size, int dx, int dy, int &x, int &y) {
  int len = getTextLen(text, size);
  x = dx - (len * size * 6 - size) / 2;
  y = dy - (size * 7) / 2;
}

int getTextLen(char *text, int size) {
  int r = 0;
  while (text[r])
    r++;
  return r;
}

void updateAll() {
  for (int i = 0; i < 10 && btns[i].c; i++) {
    btns[i].update();
  }
  bool a = textfield1.update();
  bool b = textfield2.update();
  if (a && textfield2.active) {
    textfield2.active = 0;
    textfield2.reset = 1;
  } else if (b && textfield1.active) {
    textfield1.active = 0;
    textfield1.reset = 1;
  }
  kp.update();
  fonaUpdate();
}

void fonaPickUp() {
  //fona.pickUp();
  Serial.println("FONA pick up");
}

void fonaHangUp() {
  //fona.hangUp();
  Serial.println("FONA hang up");
}

void handleCall() { // ISR
  callFlag = 1;
}

void fonaCall(char *n) {
  //fona.callPhone(n);
  Serial.print("Call to ");
  Serial.println(n);
}

void fonaSMS(char *a, char *m) {
  //fona.sendSMS(a, m);
  Serial.print("SMS to ");
  Serial.print(a);
  Serial.print(": ");
  Serial.println(m);
}

void fonaUpdate() {
  if (callFlag) {
    callFlag = 0;
    // Handle the call/SMS properly
  }
}

