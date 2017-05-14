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
#define olive         ILI9341_OLIVE

#define SCREEN_HOME   0
#define SCREEN_MENU   1
#define SCREEN_LOCK   2
#define SCREEN_PHONE  3
#define SCREEN_SMS1   4
#define SCREEN_SMS2   5
#define SCREEN_SET    6
#define SCREEN_PONG   7
#define SCREEN_RADIO  8
#define SCREEN_CALC   9
#define SCREEN_CONT   10

#define NUMPAD_W  140
#define NUMPAD_H  190

int bl = 100;

char* appNames[] = {"Phone", "SMS", "Set.", "Cont.", "PONG", "Radio", "Calc", "..."};
int appColor[] = {red, green, darkgrey, blue, black, cyan, orange, olive};
byte numOfApps = 8;
#define appHeight 64 // Screen height (320) / 5

int tX;
int tY;
int oldX;
int oldY;
TS_Point TP;

int i = 0;
int a = 0;

byte openApp = SCREEN_MENU;

char kpC[] = {'1', '2', '3',
              '4', '5', '6',
              '7', '8', '9',
              '+', '0', '#'};
              
byte kpX[] = {18, 61, 104,
              18, 61, 104,
              18, 61, 104,
              18, 61, 104};
              
byte kpY[] = {20, 20, 20,
              65, 65, 65,
              105,105,105,
              150,150,150};

void setup() {
  Serial.begin(9600);
  tft.begin();
  analogWrite(TFT_BL, bl);
  tft.fillScreen(navy);
  tft.setTextSize(1);
  tft.setTextColor(white, navy);
  tft.print(F("Starting touchscreen... "));
  ts.begin();
  tft.println(F("Done"));
  tft.print(F("Initializing SD library... "));
  SD.begin(4);
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
  draw(SCREEN_MENU, 0);
  tone(45, 600, 30);
  delay(40);
  tone(45, 600, 200);
}

void loop() {
  if (ts.touched()) {
    touchHandler(SCREEN_MENU);
  }
}

void touchHandler(byte screen) {
  int scrolled = 0;
  switch (screen) {
    case SCREEN_HOME:
      break;
    case SCREEN_MENU:
      if (getX() < 120) {
        if (getY() < appHeight * 2 && getY() > appHeight) {
          drawTRect(0, appHeight, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_PHONE, 1);
        } else if (getY() < appHeight * 3 && getY() > appHeight * 2) {
          drawTRect(0, appHeight * 2, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_SET, 1);
        } else if (getY() < appHeight * 4 && getY() > appHeight * 3) {
          drawTRect(0, appHeight * 3, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_PONG, 1);
        } else if (getY() > appHeight * 4) {
          drawTRect(0, appHeight * 4, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_CALC, 1);
        }
      } else {
        if (getY() < appHeight * 2 && getY() > appHeight) {
          drawTRect(120, appHeight, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_SMS1, 1);
        } else if (getY() < appHeight * 3 && getY() > appHeight * 2) {
          drawTRect(120, appHeight * 2, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_CONT, 1);
        } else if (getY() < appHeight * 4 && getY() > appHeight * 3) {
          drawTRect(120, appHeight * 3, 120, appHeight, navy, 5);
          while (ts.touched());
          draw(SCREEN_RADIO, 1);
        } else if (getY() > appHeight * 4) {
          drawTRect(120, appHeight * 4, 120, appHeight, navy, 5);
          while (ts.touched());
          //draw(SCREEN_CALC, 1);
        }
      }
      break;
    case SCREEN_LOCK:
      break;
    case SCREEN_PHONE:
      break;
    case SCREEN_SMS1:
      break;
    case SCREEN_SMS2:
      break;
    case SCREEN_SET:
      break;
  }
}

void draw(byte screen, bool doAnim) {
  int px1 = 0;
  int py1 = 0;
  int px2 = 0;
  int py2 = 0;
  int px3 = 0;
  int py3 = 0;
  int px4 = 0;
  int py4 = 0;
  switch (screen) {
    case SCREEN_HOME:
      openApp = SCREEN_HOME;
      break;
    case SCREEN_MENU:
      openApp = SCREEN_MENU;
      tft.fillRect(0, 0, 240, appHeight, navy);
      int tempX;
      int color;
      tft.setTextColor(white);
      tft.setTextSize(2);
      for (i = 0; i < numOfApps / 2; i++) {
        tft.fillRect(0, appHeight * (i + 1), 120, appHeight, appColor[color]);
        tft.setCursor(getTextCenter(appNames[color], 60, 2), appHeight * (i + 1) + 20);
        tft.print(appNames[color]);
        color++;
        tft.fillRect(120, appHeight * (i + 1), 120, appHeight, appColor[color]);
        tft.setCursor(getTextCenter(appNames[color], 180, 2), appHeight * (i + 1) + 20);
        tft.print(appNames[color]);
        color++;
        tft.drawFastHLine(0, (appHeight * (i + 1)) - 1, 240, black);
      }
      tft.drawFastVLine(120, appHeight, appHeight * 4, black);
      break;
    case SCREEN_LOCK:
      openApp = SCREEN_LOCK;
      break;
    case SCREEN_PHONE:
      openApp = SCREEN_PHONE;
      /*if (doAnim) {
        for (py1 = appHeight; py1 <= 320; py1 += 10) {
          px1 = 0;
          px2 = map(py1, appHeight, 320, 120, 0);
          py2 = map(py1, appHeight, 320, appHeight, 0);
          px3 = map(py1, appHeight, 320, 120, 240);
          py3 = map(py1, appHeight, 320, appHeight * 2, 0);
          px4 = map(py1, appHeight, 320, 0, 240);
          py4 = map(py1, appHeight, 320, appHeight * 2, 320);
          tft.drawLine(px1, py1, px2, py2, red);
          tft.drawLine(px2, py2, px3, py3, red);
          tft.drawLine(px3, py3, px4, py4, red);
          tft.drawLine(px4, py4, px1, py1, red);
        }
      }*/
      tft.fillScreen(white);
      tft.fillRect(0, 0, 240, 20, red);
      drawTRect(10, 30, 220, 60, black, 3);
      drawNumpad(0, 130);
      tft.fillRect(NUMPAD_W, 130, 100, 63, green);
      tft.fillRect(NUMPAD_W, 193, 100, 64, red);
      tft.fillRect(NUMPAD_W, 257, 100, 63, lightgrey);
      drawTRect(NUMPAD_W, 130, 100, 63, darkgreen, 3);
      drawTRect(NUMPAD_W, 193, 100, 64, maroon, 3);
      drawTRect(NUMPAD_W, 257, 100, 63, darkgrey, 3);
      tft.setTextSize(3);
      tft.setTextColor(darkgreen);
      tft.setCursor(getTextCenter("CALL", NUMPAD_W + 50, 3), getTextCenter(162, 3));
      tft.print("CALL");
      tft.setTextColor(maroon);
      tft.setCursor(getTextCenter("RMV", NUMPAD_W + 50, 3), getTextCenter(225, 3));
      tft.print("RMV");
      tft.setTextColor(darkgrey);
      tft.setCursor(getTextCenter("CLEAR", NUMPAD_W + 50, 3), getTextCenter(289, 3));
      tft.print("CLEAR");
      break;
    case SCREEN_SMS1:
      openApp = SCREEN_SMS1;
      break;
    case SCREEN_SMS2:
      openApp = SCREEN_SMS2;
      break;
    case SCREEN_SET:
      openApp = SCREEN_SET;
      break;
  }
}

void drawNumpad(byte x, byte y) {
  drawTRect(x, y, NUMPAD_W, NUMPAD_H, black, 3);
  tft.setTextColor(black);
  tft.setTextSize(3);
  for (int i = 0; i < 12; i++) {
    tft.setCursor(x + kpX[i], y + kpY[i]);
    tft.print(kpC[i]);
  }
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
  Serial.println(length);
  return length;
}

int getChars(char text[]) {
  String tempText = String(text);
  return tempText.length();
}

int getTextCenter(char text[], int x, int font) {
  return x - getTextLength(text, font) / 2;
}

int getTextCenter(int y, int font) {
  return y - 3.5 * font;
}

void drawTRect(int x, int y, int w, int h, int color, int t) {
  for (i = 0; i < t; i++) {
    tft.drawRect(x + i, y + i, w - i * 2, h - i * 2, color);
  }
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
