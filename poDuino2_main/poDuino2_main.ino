#include <SD.h> // Libraries
#include <Adafruit_FONA.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <SoftwareSerial.h>

Adafruit_FT6206 TS = Adafruit_FT6206(); // Library initializations

#define TFT_CS 10
#define TFT_DC 9
#define SD_CS 4
#define TFT_BL 5
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 7
#define FONA_RI 6
#define FONA_KEY 8
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

#define lightgrey ILI9341_LIGHTGREY // System definitions/variables
#define black ILI9341_BLACK
#define blue ILI9341_BLUE
#define navy ILI9341_NAVY
#define white ILI9341_WHITE

#define iconSize 40

#define LSTextX 140
#define LSTextY 65

int password[] = {0, 0, 0, 0};

int givenPassword[] = {NULL, NULL, NULL, NULL};

int bl = 2;

bool phoneLocked = true;

// Applications/screens: 0 is lockscreen, 1 is menu, 2 is settings

void setup() {
  pinMode(FONA_RI, INPUT);
  pinMode(FONA_KEY, OUTPUT);
  pinMode(TFT_BL, OUTPUT);
  backlight(bl);
  tft.begin();
  tft.setTextColor(black);
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  if (!SD.begin(SD_CS)) {
    tft.println(F("No SD card!"));
    tft.setCursor(5, 35);
    tft.println(F("Please insert a SD"));
    tft.setCursor(5, 65);
    tft.print(F("card and reboot."));
    while(true){}
  }
  SD.begin(SD_CS);
  bmpDraw("start.bmp", 0, 0);
  TS.begin(40);
  fonaSerial->begin(4800);
}

void loop() {
  if (phoneLocked) {
    tft.setTextSize(1);
    tft.setTextColor(black, white);
    bmpDraw("LS.bmp", 0, 0);
    while (phoneLocked) {
      if (TS.touched()) {
        touchHandler(0);
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
        }
      }
      while (!TS.touched()) {}
    }
  }
}

void backlight(int a) {
  int b = map(a, 0, 10, 0, 255);
  analogWrite(TFT_BL, b);
}

void touchHandler(int a) {
  TS_Point touchPoint = TS.getPoint();
  touchPoint.x = map(touchPoint.x, 0, 240, 240, 0);
  touchPoint.y = map(touchPoint.y, 0, 320, 320, 0);
  switch (a) {
    case 0:
      if (touchPoint.x >= 35 && touchPoint.y >= 50 && touchPoint.x <= 210 && touchPoint.y <= 215) {
        if (touchPoint.x >= 35 && touchPoint.y >= 50 && touchPoint.x <= 75 && touchPoint.y <= 95) {
          insertToGivenPassword(1);
        } else if (touchPoint.x >= 95 && touchPoint.y >= 50 && touchPoint.x <= 150 && touchPoint.y <= 95) {
          
        }
      } else if (touchPoint.x >= 35 && touchPoint.y >= 225 && touchPoint.x <= 80 && touchPoint.y <= 270) {
        insertToGivenPassword(0);
      }
      break;
  }
}

void insertToGivenPassword(int a) {
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

bool checkIfPasswordCorrect() {
  if (givenPassword[0] == password[0] && givenPassword[1] == password[1] && givenPassword[2] == password[2] && givenPassword[3] == password[3]) {
    return true;
  } else {
    return false;
  }
}

//----------------------------- BITMAP DRAWING -------------------------------
//                         (Function from Adafruit)

#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("BMP format not recognized."));
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





