#include <SD.h> // Libraries
#include <Adafruit_FONA.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Adafruit_FT6206.h>
#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>

Adafruit_FT6206 ctp = Adafruit_FT6206(); // Library initializations

#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 7
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
#define FONA_RI 6
#define FONA_KEY 8
#define SD_CS 4
#define TFT_BL 5

#define lightgrey ILI9341_LIGHTGREY // Definitions and variables
#define black ILI9341_BLACK
#define blue ILI9341_BLUE
#define navy ILI9341_NAVY
#define white ILI9341_WHITE

#define iconSize 40
#define iconsPerRow 3
#define clockX (tft.width() / 2) - 10
#define clockY 1

#define popupX 40
#define popupY 60

int tpX;

long tsc;
TS_Point ts;

uint16_t vbat;

int openApp = 1; // 0 = lockscreen, 1 = menu, 2 = settings
int openedApp = 0; // Same here

char replybuffer[255];

int curX = 0;
int curY = 0;

String apps[] = {"S"};
int iconAmount = (sizeof(apps) / sizeof(int)) - 1;

char* icons[] = {"settings_icon.bmp"};
int iconX[] = {15, 80, 160};
int iconY[] = {20, 70, 120};

int bl = 8;

int btr;

int topbarOld = 0;

void setup() {
  pinMode(FONA_RI, INPUT);
  pinMode(FONA_KEY, OUTPUT);

  tft.begin();
  SD.begin(SD_CS);
  bmpDraw("start_logo.bmp", 0, 0);
  delay(100);
  backlight(bl);
  delay(100);
  ctp.begin(40);
  delay(100);
  fonaSerial->begin(4800);
  delay(100);
  splashScreen(1000);
}

void loop() {
  draw(openedApp);
  btr = fona.getBattPercent(&vbat);
  if (btr <= 4) {
    popup("Battery too low...", 1);
    sleep();
  } else if (btr <= 10) {
    popup("Battery low...\n(10% left)", 1);
    sleep();
  } else if (btr <= 20) {
    popup("Battery low...\n(20% left)", 1);
    sleep();
  }
  updateTopbar();
  if (ctp.touched()) {
    touchHandler(1);
  }
}

void sleep() {
  for (int i = bl; i > 0; i--) {
    backlight(i);
    delay(3);
  }
  FONApwrToggle(0);
  while (! ctp.touched()) {}
  wakeup();
}

void backlight(int b) {
  int c = map(b, 0, 10, 0, 255);
  analogWrite(TFT_BL, c);
}

void wakeup() {
  draw(openApp);
  for (int i = 0; i < bl; i++) {
    backlight(i);
    delay(3);
  }
}

void popup(String g, int numBtns) {
  if (numBtns == 1) {
    bmpDraw("popup1.bmp", popupX, popupY);
    tft.setCursor((popupX + 5), (popupY + 5));
    tft.print(g);
    while (! ctp.touched()) {}
  } else if (2) {
    bmpDraw("popup2.bmp", popupX, popupY);
    tft.setCursor((popupX + 5), (popupY + 5));
    tft.print(g);
    while (! ctp.touched()) {}
    ts = ctp.getPoint();
    tpX = ts.x;
  }
  closePopup();
}

void FONApwrToggle(int f) {
  int FONApwrState;
  if (fona.setFMVolume(5)) {
    FONApwrState = 1;
  } else {
    FONApwrState = 0;
  }
  if (f == 0) {
    if (FONApwrState == 1) {
      digitalWrite(FONA_KEY, LOW);
      delay(2010);
      digitalWrite(FONA_KEY, HIGH);
    }
  } else if (f == 1) {
    if (FONApwrState == 0) {
      digitalWrite(FONA_KEY, LOW);
      delay(2010);
      digitalWrite(FONA_KEY, HIGH);
    }
  }
}

void closePopup() {
  draw(openApp);
}

void draw(int p) {
  if (p != openApp) {
    switch (p) {
      case 0: bmpDraw("lockscreen.bmp", 0, 0); break;
      case 1: drawMenu(); break;
    }
    openApp = p;
  }
}

void drawMenu() {
  bmpDraw("menu.bmp", 0, 0);
  drawMenuIcons();
}

void drawMenuIcons() {
  int j = 0;
  for (int i = 0; i < iconAmount; i++) {
    j = i;
    while (j <= 4) {
      j = (j - 3);
    }
    bmpDraw(icons[i], iconX[j], iconY[j]);
  }
}

void updateTopbar() {
  if (topbarOld == 0) {

  }
  updateTime();
  drawBattery();
}

void updateTime() {
  /*times[0] = getHours(); // Time in hours
    times[1] = getMinutes(); // Time in seconds */
  tft.setTextColor(black);
  tft.setTextSize(2);
  tft.setCursor(clockX, clockY);
  tft.print("11:11"); // print times[0], ":" and times[1]
}

void drawBattery() {
  btr = fona.getBattPercent(&vbat);
  if (btr < 20) {
    bmpDraw("battery1.bmp", 210, 1);
  } else if (btr < 40) {
    bmpDraw("battery2.bmp", 210, 1);
  } else if (btr < 60) {
    bmpDraw("battery3.bmp", 210, 1);
  } else if (btr < 80) {
    bmpDraw("battery4.bmp", 210, 1);
  } else if (btr <= 100) {
    bmpDraw("battery5.bmp", 210, 1);
  }
}

void touchHandler(int a) {
  if (a == 0) {

  } else if (a == 1) {
    TS_Point ts = ctp.getPoint();
    if (ts.x >= iconX[1] && ts.x <= (iconX[1] + iconSize)) {
      settings();
    }
    if (tsc >= 0 && tsc <= 0) {
      // App 3
    }
    if (tsc >= 0 && tsc <= 0) {
      // App 4
    }
    if (tsc >= 0 && tsc <= 0) {
      // App 5...
    }
  }
}

void settings() {
  
}

void locksreen() {

}

void splashScreen(int wait) {
  tft.fillScreen(ILI9341_GREEN);
  tft.setCursor(((tft.width() / 2) - 10), ((tft.height() / 2) - 10));
  tft.setTextColor(black);
  tft.println("poDuino 2nd Generation,");
  delay(100);
  tft.println("(now called 'Nano',\nrunning on 'NanOS')");
  delay(100);
  tft.println("by Nanohenry");
  delay(wait);
  tft.print("Hehehe, the SS was the old SS");
  delay(500);
}

String SDread(String fileToRead) {
  String returnValue;
  if (File curFile = SD.open(fileToRead)) {
    returnValue = curFile.read();
    curFile.close();
  } else {
    returnValue = "Error";
  }
  return returnValue;
}

void SDwrite(String fileToWrite, String toWrite) {
  if (File curFile = SD.open(fileToWrite)) {
    curFile.print(toWrite);
    curFile.close();
  }
}



//----------------------------- BITMAP DRAWING -------------------------------

#define BUFFPIXEL 20 // For drawing bimaps

void bmpDraw(char *filename, uint8_t x, uint16_t y) {

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

  if ((x >= tft.width()) || (y >= tft.height())) return;

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    return;
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // BMP signature
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    // Read DIB header
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!

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
      } // end goodBmp
    }
  }

  bmpFile.close();
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





