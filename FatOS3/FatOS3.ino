#include <avr/wdt.h>

#include <Adafruit_FONA.h>
#include <SoftwareSerial.h>

#define FONA_RX 2
#define FONA_TX 12
#define FONA_RST 4
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint16_t returnVal;

#include <SMARTGPU2.h>

SMARTGPU2 tft;

VIDDATA metaVideo;
POINT tPoint;

FILENAME lock_wp = "default_wp";
FILENAME menu_wp = "default_wp";

#define LOCK 0

#define KP_C 1
#define KP_N 2

#define MENU 3
#define APP_SET 4
#define APP_PHONE 5
#define APP_SMS 6
#define APP_RADIO 7

int appAmount = 5;

byte openApp = MENU;
byte screen = 0;

char startText[] = {"STARTING"};
int colouredLetter;
int startTextX[] = {80, 100, 120, 140, 160, 185, 200, 220};

int i = 0;

uint16_t FONAbatt = 0;

long int updateTimer = millis();

bool crash = false;

void setup() {
  wdt_reset();
  Serial.begin(9600);
  startTFT();
  initTFT(10);
  tft.bright(0);
  drawTFTBMP("starting", 0, 0);
  for (i = 0; i <= 100; i++) {
    tft.bright(i);
    delay(10);
  }

  //startFONA();

  for (i = 100; i >= 0; i--) {
    tft.bright(i);
    delay(2);
  }
  draw(MENU);
  for (i = 0; i <= 100; i++) {
    tft.bright(i);
    delay(2);
  }

  cli();
  wdt_reset();
  WDTCSR |= 0b00011000; // Watchdog edit mode
  WDTCSR = 0b01000000 | 0b100000; // Set watchdog [INT, RESET | DELAY]
  sei();
  //  16 ms:     0b000000
  //  500 ms:    0b000101
  //  1 second:  0b000110
  //  2 seconds: 0b000111
  //  4 seconds: 0b100000
  //  8 seconds: 0b100001

}

void loop() {
  appRoutine(BLUE); // Draw top clock, battery etc.
  wdt_reset(); // Reset watchdog
  checkCrash();
}

void draw(byte screen) {
  switch (screen) {
    case LOCK:
      break;
    case MENU:
      drawTFTBMP(menu_wp, 0, 0);
      for (i = 0; i < appAmount; i++) {
        if (i == 0) {
          drawTFTBMP("phoneIcon", 16, 70);
        } else if (i == 1) {
          drawTFTBMP("SMSIcon", 92, 70);
        } else if (i == 2) {
          drawTFTBMP("musicIcon", 168, 70);
        } else if (i == 3) {
          drawTFTBMP("setIcon", 244, 70);
        } else if (i == 4) {
          drawTFTBMP("photosIcon", 16, 150);
        }
      }
      drawTopHeader(BLUE);
      break;
    case KP_C:
      break;
    case KP_N:
      break;
    case APP_SET:
      break;
    case APP_PHONE:
      for (i = 319; i >= 0; i--) {
        tft.drawLine(i, 0, i, 479, BLUE);
        delay(1);
      }
      drawText("Phone", 110, 210, FONT5, WHITE, false);
      delay(2000);
      drawTFTBMP("phone1", 0, 0);
      drawTFTBMP("topRed", 0, 0);
      checkDrawBattery();
      break;
    case APP_SMS:
      break;
    case APP_RADIO:
      break;
    default:
      break;
  }
}

void call() {
  drawTFTBMP("call1", 0, 0);
}

void call(char* number) {
  drawTFTBMP("call2noMute", 0, 0);
}

ISR(WDT_vect) {
  crash = 1;
}

void checkCrash() {
  if (crash) {
    tft.drawRectangle(0, 0, 319, 479, BLUE, (FILLGEOM)true);
    tft.setTextColour(WHITE);
    tft.setTextSize(FONT4);
    tft.setTextBackFill(TRANS);
    tft.string(20, 30, 319, 479, "FatOS has", 0);
    tft.string(20, 70, 319, 479, "crashed.", 0);
    tft.string(20, 110, 319, 479, "Please roboot by", 0);
    tft.string(20, 150, 319, 479, "pressing RESET.", 0);
    delay(2000);
    tft.string(20, 230, 319, 479, "Don't worry, this", 0);
    tft.string(20, 270, 319, 479, "won't happen", 0);
    tft.string(20, 310, 319, 479, "very often. :D", 0);
    delay(10000);
    tft.string(20, 390, 319, 479, "C'mon, hit the", 0);
    tft.string(20, 430, 319, 479, "RESET button!", 0);
    TFTWAV("error", 0);
    while (1);
  }
}

void appRoutine(int color) {
  if (millis() - updateTimer >= 10000) {
    drawTopHeader(color);
    updateTimer = millis();
  }
  checkTouch();
}

void checkTouch() {
  tft.touchScreen(&tPoint);
  switch (openApp) {
    case MENU:
      if (tPoint.x >= 16 && tPoint.y >= 70 && tPoint.x <= (16 + 60) && tPoint.y <= (70 + 60)) {
        phoneApp();
      }
      break;
  }
}

void phoneApp() {
  openApp = APP_PHONE;
  screen = APP_PHONE;
  draw(APP_PHONE);
  while (tPoint.y > 20) {
    tft.touchScreen(&tPoint);
    checkCrash();
    appRoutine(RED);
  }
  exitApp();
}

void exitApp() {
  openApp = MENU;
  screen = MENU;
  draw(MENU);
}

void drawTopHeader(int color) {
  if (color == RED) {
    drawTFTBMP("topRed", 0, 0);
  } else if (color == GREEN) {
    drawTFTBMP("topGreen", 0, 0);
  } else if (color == BLUE) {
    drawTFTBMP("topBlue", 0, 0);
  }
  checkDrawBattery();
}

void checkDrawBattery() {
  //FONAbatt = getBtry();
  if (FONAbatt >= 90) {
    drawTFTBMP("batt100", 2, 2);
  } else if (FONAbatt < 90 && FONAbatt >= 65) {
    drawTFTBMP("batt75", 2, 2);
  } else if (FONAbatt < 65 && FONAbatt >= 40) {
    drawTFTBMP("batt50", 2, 2);
  } else if (FONAbatt < 40 && FONAbatt >= 15) {
    drawTFTBMP("batt25", 2, 2);
  } else if (FONAbatt < 15 && FONAbatt >= 5) {
    drawTFTBMP("batt10", 2, 2);
    alert("Battery Low");
  } else if (FONAbatt < 5 && FONAbatt >= 0) {
    drawTFTBMP("batt0", 2, 2);
    alert("Battery Very Low");
  }
}

bool startFONA() {
  fonaSerial->begin(4800);
  return fona.begin(*fonaSerial);
}

int getBtry() {
  fona.getBattPercent(&returnVal);
  return int(returnVal);
}

void startTFT() {
  tft.init();
  tft.start();
}

void initTFT(int vol) {
  tft.baudChange(BAUD6);
  tft.orientation(PORTRAIT_LOW);
  tft.initDACAudio(ENABLE);
  tft.audioBoost(ENABLE);
  tft.setVolumeWAV(vol);
}

void drawTFTBMP(FILENAME fileN, AXIS imageX, AXIS imageY) {
  tft.imageBMPSD(imageX, imageY, fileN);
}

void alert(char* text) {
  tft.imageBMPSD(10, 140, "alertBox");
  tft.setTextColour(WHITE);
  tft.setTextBackFill(TRANS);
  tft.setTextSize(FONT3);
  tft.string(30, 160, 300, 330, text, 0);
  while (tft.touchScreen(&tPoint) == INVALID) {
    wdt_reset();
  }
}

void TFTWAV(FILENAME fileName, unsigned int* a) {
  tft.playWAVFile(fileName, a);
}

void drawText(char* text, int x, int y, FONTSIZE font, int color, bool cen) {
  tft.setTextColour(color);
  tft.setTextSize(font);
  if (cen) {
    tft.string(x, y, 319, 479, text, 0); // Center text???
  } else {
    tft.string(x, y, 319, 479, text, 0);
  }
}









