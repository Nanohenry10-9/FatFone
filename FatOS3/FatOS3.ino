#include <avr/wdt.h>

#include <Adafruit_FONA.h>
#include <SoftwareSerial.h>

#define FONA_RX 2
#define FONA_TX 12
#define FONA_RST 4
#define FONA_RI 6
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
#define APP_MUSIC 7
#define APP_PHOTOS 8

int appAmount = 5;

byte openApp = MENU;
byte screen = MENU;

char startText[] = {"STARTING"};
int colouredLetter;
int startTextX[] = {80, 100, 120, 140, 160, 185, 200, 220};

int i = 0;

uint16_t FONAbatt = 0;

int audio = FONA_EXTAUDIO;
int vol = 75;

long int updateTimer = millis();

bool crash = false;

long battTimer = millis();

bool allowAlert = false;

unsigned int files = 0;
unsigned int dirs = 0;
char songBuff[] = {" "};
int songY[] = {70, 130, 190, 250, 310};
bool pause = false;

char dialNumber[14] = {" "};
int curNum = 0;
char numBuff[20] = {" "};
float callTimer = 0;
char cCallTimer[5] = {' '};
String timerBuff;

bool con = false;

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

  startFONA();
  Serial.println(F("FONA started"));

  for (i = 100; i >= 0; i--) {
    tft.bright(i);
    delay(2);
  }
  draw(MENU);
  TFTWAV("startSound", 0);
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
  Serial.println(F("WDT enabled"));
  allowAlert = true;

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
      tft.drawRectangle(0, 0, 319, 479, WHITE, (FILLGEOM)true);
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
        tft.drawLine(i, 0, i, 479, RED);
        delay(1);
      }
      drawText("Phone", 110, 210, FONT5, WHITE, false);
      delay(2000);
      drawTFTBMP("phone1", 0, 0);
      drawTFTBMP("topRed", 0, 0);
      checkDrawBattery();
      break;
    case APP_SMS:
      for (i = 319; i >= 0; i--) {
        tft.drawLine(i, 0, i, 479, GREEN);
        delay(1);
      }
      drawText("Messages", 60, 210, FONT5, WHITE, false);
      delay(2000);
      drawTFTBMP("musicRadio", 0, 0);
      drawTopHeader(GREEN);
      break;
    case APP_MUSIC:
      for (i = 319; i >= 0; i--) {
        tft.drawLine(i, 0, i, 479, BLUE);
        delay(1);
      }
      drawText("Music", 110, 210, FONT5, WHITE, false);
      delay(2000);
      drawTFTBMP("music1", 0, 0);
      drawTopHeader(BLUE);
      listMusic();
      break;
    default:
      break;
  }
}

void call() {
  drawTFTBMP("phone3", 0, 0);
  drawText(numBuff, 20, 30, FONT3, BLACK, false);
  while (!con) {
    if (tft.touchScreen(&tPoint) == VALID) {
      if (tPoint.y >= 290) {
        fona.hangUp();
        con = true;
      } else if (tPoint.y >= 125 && tPoint.y <= 290) {
        drawTFTBMP("phone2", 0, 0);
        fona.pickUp();
        callTimer = millis();
        drawText(numBuff, 20, 30, FONT3, BLACK, false);
        while (!con) {
          if (tft.touchScreen(&tPoint) == VALID) {
            if (tPoint.y >= 290) {
              con = true;
            }
          }
          checkCrash();
          wdt_reset();
        }
        con = false;
        fona.hangUp();
        drawText("Call ended, duration:", 20, 60, FONT3, BLACK, false);
        timerBuff = callTimer / 1000;
        timerBuff.toCharArray(cCallTimer, 5);
        drawText(cCallTimer, 20, 90, FONT3, BLACK, false);
        delay(1000);
        con = true;
      }
    }
    checkCrash();
    wdt_reset();
  }
  con = false;
}

void call(char* number) {
  drawTFTBMP("phone2", 0, 0);
  callTimer = millis();
  fona.callPhone(number);
  drawText(number, 20, 30, FONT3, BLACK, false);
  while (!con) {
    if (tft.touchScreen(&tPoint) == VALID) {
      if (tPoint.y >= 290) {
        con = true;
      }
    }
    checkCrash();
    wdt_reset();
  }
  con = false;
  fona.hangUp();
  drawText("Call ended, duration:", 20, 60, FONT3, BLACK, false);
  timerBuff = callTimer / 1000;
  timerBuff.toCharArray(cCallTimer, 5);
  drawText(cCallTimer, 20, 90, FONT3, BLACK, false);
  delay(1000);
  drawTFTBMP("phone1", 0, 0);
}

ISR(WDT_vect) {
  crash = 1;
  fona.hangUp();
  fona.setPWM(0);
}

void checkCrash() {
  if (crash) {
    Serial.println(F("(!) WDT interrupt (!)"));
    tft.stopWAVFile();
    TFTWAV("error", 0);
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
    while (1);
  }
}

void appRoutine(int color) {
  if (millis() - updateTimer >= 10000) {
    drawTopHeader(color);
    updateTimer = millis();
  }
  checkTouch();
  if (fona.incomingCallNumber(numBuff)) {
    call();
  }
}

void checkTouch() {
  tft.touchScreen(&tPoint);
  switch (openApp) {
    case MENU:
      if (tPoint.x >= 16 && tPoint.y >= 70 && tPoint.x <= (16 + 60) && tPoint.y <= (70 + 60)) {
        phoneApp();
      } else if (tPoint.x >= 92 && tPoint.y >= 70 && tPoint.x <= (92 + 60) && tPoint.y <= (70 + 60)) {
        SMSapp();
      } else if (tPoint.x >= 168 && tPoint.y >= 70 && tPoint.x <= (168 + 60) && tPoint.y <= (70 + 60)) {
        musicApp();
      } else if (tPoint.x >= 168 && tPoint.y >= 70 && tPoint.x <= (168 + 60) && tPoint.y <= (70 + 60)) {
        setApp();
      } else if (tPoint.x >= 168 && tPoint.y >= 70 && tPoint.x <= (168 + 60) && tPoint.y <= (70 + 60)) {
        imagesApp();
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
    wdt_reset();
    if (tft.touchScreen(&tPoint) == VALID) {
      if (tPoint.x >= 0 && tPoint.y >= 0 && tPoint.x <= 160 && tPoint.y <= 180) {
        tft.drawRectangle(6, 195, 313, 265, WHITE, (FILLGEOM)true);
        call(dialNumber);
        for (i = 0; i < 13; i++) {
          dialNumber[i] = ' ';
        }
        curNum = 0;
      } else if (tPoint.x >= 160 && tPoint.y >= 0 && tPoint.x <= 320 && tPoint.y <= 180) {
        // Contacts
      } else if (tPoint.x >= 0 && tPoint.y >= 273 && tPoint.x <= 76 && tPoint.y <= 349) {
        if (curNum < 13) {
          dialNumber[curNum] = '1';
          curNum++;
          fona.playDTMF('1');
        }
      } else if (tPoint.x >= 76 && tPoint.y >= 273 && tPoint.x <= 155 && tPoint.y <= 349) {
        if (curNum < 13) {
          dialNumber[curNum] = '2';
          curNum++;
          fona.playDTMF('2');
        }
      } else if (tPoint.x >= 155 && tPoint.y >= 273 && tPoint.x <= 230 && tPoint.y <= 349) {
        if (curNum < 13) {
          dialNumber[curNum] = '3';
          curNum++;
          fona.playDTMF('3');
        }
      } else if (tPoint.x >= 230 && tPoint.y >= 273 && tPoint.x <= 320 && tPoint.y <= 349) {
        if (curNum < 13) {
          dialNumber[curNum] = '4';
          curNum++;
          fona.playDTMF('4');
        }
      } else if (tPoint.x >= 0 && tPoint.y >= 349 && tPoint.x <= 76 && tPoint.y <= 410) {
        if (curNum < 13) {
          dialNumber[curNum] = '5';
          curNum++;
          fona.playDTMF('5');
        }
      } else if (tPoint.x >= 76 && tPoint.y >= 349 && tPoint.x <= 155 && tPoint.y <= 410) {
        if (curNum < 13) {
          dialNumber[curNum] = '6';
          curNum++;
          fona.playDTMF('6');
        }
      } else if (tPoint.x >= 155 && tPoint.y >= 349 && tPoint.x <= 230 && tPoint.y <= 410) {
        if (curNum < 13) {
          dialNumber[curNum] = '7';
          curNum++;
          fona.playDTMF('7');
        }
      } else if (tPoint.x >= 230 && tPoint.y >= 349 && tPoint.x <= 320 && tPoint.y <= 410) {
        if (curNum < 13) {
          dialNumber[curNum] = '8';
          curNum++;
          fona.playDTMF('8');
        }
      } else if (tPoint.x >= 0 && tPoint.y >= 410 && tPoint.x <= 76 && tPoint.y <= 480) {
        if (curNum < 13) {
          dialNumber[curNum] = '9';
          curNum++;
          fona.playDTMF('9');
        }
      } else if (tPoint.x >= 76 && tPoint.y >= 410 && tPoint.x <= 155 && tPoint.y <= 480) {
        if (curNum < 13) {
          dialNumber[curNum] = '0';
          curNum++;
          fona.playDTMF('0');
        }
      } else if (tPoint.x >= 155 && tPoint.y >= 410 && tPoint.x <= 230 && tPoint.y <= 480) {
        if (curNum < 13) {
          dialNumber[curNum] = '+';
          curNum++;
          fona.playDTMF('2');
        }
      } else if (tPoint.x >= 230 && tPoint.y >= 410 && tPoint.x <= 320 && tPoint.y <= 480) {
        if (curNum > 0) {
          curNum--;
          dialNumber[curNum] = ' ';
          fona.playDTMF('6');
        }
      }
      if (tPoint.y > 273) {
        tft.drawRectangle(6, 195, 313, 265, WHITE, (FILLGEOM)true);
        drawText(dialNumber, 6, 210, FONT5, BLACK, false);
      }
      delay(300);
    }
  }
  exitApp();
}

void SMSapp() {
  openApp = APP_SMS;
  screen = APP_SMS;
  draw(APP_SMS);
  while (tPoint.y > 20) {
    tft.touchScreen(&tPoint);
    checkCrash();
    appRoutine(GREEN);
    wdt_reset();
  }
  exitApp();
}

void musicApp() {
  openApp = APP_MUSIC;
  screen = APP_MUSIC;
  draw(APP_MUSIC);
  while (tPoint.y > 20) {
    tft.touchScreen(&tPoint);
    checkCrash();
    appRoutine(BLUE);
    wdt_reset();
    if (tft.touchScreen(&tPoint) == VALID) {
      tft.SDFgetList(&dirs, &files);
      if (files >= 1 && tPoint.y > (songY[0] - 10) && tPoint.y < (songY[0] + 20)) {
        while (tft.touchScreen(&tPoint) == VALID) {}
        WAVPlayer(0);
      } else if (files >= 2 && tPoint.y > (songY[1] - 10) && tPoint.y < (songY[1] + 20)) {
        while (tft.touchScreen(&tPoint) == VALID) {}
        WAVPlayer(1);
      } else if (files >= 3 && tPoint.y > (songY[2] - 10) && tPoint.y < (songY[2] + 20)) {
        while (tft.touchScreen(&tPoint) == VALID) {}
        WAVPlayer(2);
      } else if (files >= 4 && tPoint.y > (songY[3] - 10) && tPoint.y < (songY[3] + 20)) {
        while (tft.touchScreen(&tPoint) == VALID) {}
        WAVPlayer(3);
      } else if (files >= 5 && tPoint.y > (songY[4] - 10) && tPoint.y < (songY[4] + 20)) {
        while (tft.touchScreen(&tPoint) == VALID) {}
        WAVPlayer(4);
      }
    }
  }
  exitApp();
}

void WAVPlayer(ITEMNUMBER number) {
  drawTFTBMP("music2", 0, 0);
  drawTopHeader(BLUE);
  drawTFTBMP("musicPause", 100, 100);
  tft.SDFopenDir("Music");
  tft.SDFgetFileName(number, songBuff);
  cutFileExtension(songBuff, ".wav");
  tft.stopWAVFile();
  TFTWAV(songBuff, 0);
  tft.SDFopenDir("..");
  while (tPoint.y >= 50) {
    tft.touchScreen(&tPoint);
    checkCrash();
    wdt_reset();
    if (tft.touchScreen(&tPoint) == VALID) {
      if (tPoint.y >= 50) {
        if (pause) {
          pause = false;
          drawTFTBMP("musicPause", 100, 100);

        } else {
          pause = true;
          drawTFTBMP("musicPlay", 100, 100);
        }
      }
    }
  }
  drawTFTBMP("music1", 0, 0);
  drawTopHeader(BLUE);
  listMusic();
}

void setApp() {
  openApp = APP_SET;
  screen = APP_SET;
  draw(APP_SET);
  while (tPoint.y > 20) {
    tft.touchScreen(&tPoint);
    checkCrash();
    appRoutine(RED);
    wdt_reset();
  }
  exitApp();
}

void imagesApp() {
  openApp = APP_PHOTOS;
  screen = APP_PHOTOS;
  draw(APP_PHOTOS);
  while (tPoint.y > 20) {
    tft.touchScreen(&tPoint);
    checkCrash();
    appRoutine(BLUE);
    wdt_reset();
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
  FONAbatt = getBtry();
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
    if (millis() - battTimer >= 30000 && allowAlert) {
      alert("Battery Low");
      battTimer = millis();
    }
  } else if (FONAbatt < 5) {
    drawTFTBMP("batt0", 2, 2);
    if (allowAlert) {
      alert("Battery Very Low (5%)\nShutting down...");
      shutDown();
    }
  }
}

bool startFONA() {
  fonaSerial->begin(4800);
  if (fona.begin(*fonaSerial)) {
    fona.callerIdNotification(true, FONA_RI);
    fona.setAudio(audio);
    fona.setVolume(vol);
    fona.setPWM(0);
    return true;
  } else {
    return false;
  }
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
  tft.baudChange(BAUD5);
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
  tft.setTextColour(BLACK);
  tft.setTextBackFill(TRANS);
  tft.setTextSize(FONT3);
  tft.string(30, 160, 300, 330, text, 0);
  while (tft.touchScreen(&tPoint) == INVALID) {
    wdt_reset();
  }
  if (FONAbatt >= 5) {
    draw(screen);
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

void shutDown() {
  drawTFTBMP("starting", 0, 0);
  // Shutdown FONA
  for (i = 100; i >= 0; i--) {
    tft.bright(i);
    delay(10);
  }
  // Cut power
  while (1) {}
}

void listMusic() {
  tft.SDFopenDir("Music");
  tft.SDFgetList(&dirs, &files);
  if (files > 5) {
    files = 5;
  }
  for (i = 0; i < files; i++) {
    tft.SDFgetFileName(i, songBuff);
    cutFileExtension(songBuff, ".wav");
    drawText(songBuff, 30, songY[i], FONT3, BLACK, false);
  }
  tft.SDFopenDir("..");
  delay(10);
}

void cutFileExtension(char *name, char *ext) {
  char *pch;
  pch = strstr(name, ext);
  strncpy(pch, 0x00, 1);
}







