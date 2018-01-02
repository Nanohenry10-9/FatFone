#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>

#include <SD.h>

#define TFT_CS 10
#define TFT_DC 9
#define TFT_BL 5
#define SD_CS 4
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#include <Adafruit_FONA.h>
#include <SoftwareSerial.h>
#define FONA_RX 2
#define FONA_TX 12
#define FONA_RST 4
#define FONA_RI 3
#define FONA_KEY A0
#define FONA_PWR A1
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

void setup() {
  Serial.begin(115200);
  tft.begin();
  if (SD.begin(SD_CS)) {
    Serial.println(F("OK: SD initialization successful"));
  } else {
    Serial.println(F("FATAL: SD initialization failed"));
    while (1);
  }
  byte exe = executeFile(F("OS/BOOT/boot.ffp"));
  if (exe == 1) {
    Serial.println(F("OK: Bootup file read successful"));
  } else if (exe == -1) {
    Serial.println(F("FATAL: Bootup file not found"));
    while (1);
  } else {
    Serial.println(F("FATAL: Bootup file read failed"));
    while (1);
  }
  //tft.print("Hello World");
}

void loop() {

}

byte executeFile(String progFile) { // An implementation of Brainfu*k with some heavy modificaions
  File program;
  if (!SD.exists(progFile)) {
    return -1;
  }
  if (!(program = SD.open(progFile))) {
    return 0;
  }
  char read = '\0';
  int a[256];
  a[0] = 0; // Weird: this has to be set
  byte p = 0;
  bool jmp = 0;
  int cb = '\0';
  byte addr = 0;
  bool isAddr = 1;
  int num = 0;
  byte addrI = 0;
  byte numI = 0;
startRead:
  while (program.available()) {
    do {
      read = program.read();
      if (read != ';') {
        if (isDigit(read) && !jmp) {
          if (isAddr) {
            addr *= 10;
            addr += read - '0';
            addrI++;
            if (addrI >= 3) {
              isAddr = 0;
              addrI = 0;
              p = addr;
              addr = 0;
            }
          } else {
            num *= 10;
            num += read - '0';
            numI++;
            if (numI >= 3) {
              numI = 0;
              a[p] = num;
              num = 0;
            }
          }
        }
        switch (read) {
          case '>':
            if (!jmp) {
              isAddr = 1;
            }
            break;
          case '+':
            if (!jmp) {
              a[p]++;
              if (p >= 4 && p <= 6 && a[p] > 10) {
                p = 0;
              }
            }
            break;
          case '-':
            if (!jmp) {
              a[p]--;
              if (p >= 4 && p <= 6 && a[p] > 10) {
                p = 0;
              }
            }
            break;
          case '[':
            if (jmp) {
              jmp = 0;
            } else if (a[p] == a[255]) {
              jmp = 1;
            }
            break;
          case ']':
            if (jmp) {
              jmp = 0;
            } else {
              jmp = 1;
              goto startRead;
            }
            break;
          case '&':
            cb = a[p];
            break;
          case '%':
            a[p] = cb;
            break;
          case '!':
            a[p] = 0;
            break;
          case 'l':
            tft.drawLine(a[0], a[1], a[2], a[3], getRGB565(a[4] * 3.1, a[5] * 6.3, a[6] * 3.1));
            break;
          case 'b':
            tft.drawRect(a[0], a[1], a[2], a[3], getRGB565(a[4] * 3.1, a[5] * 6.3, a[6] * 3.1));
            break;
          case 'f':
            tft.fillRect(a[0], a[1], a[2], a[3], getRGB565(a[4] * 3.1, a[5] * 6.3, a[6] * 3.1));
            break;
          case 'c':
            tft.setCursor(a[0], a[1]);
            tft.setTextSize(a[2]);
            tft.setTextColor(getRGB565(a[4] * 3.1, a[5] * 6.3, a[6] * 3.1)/*, getRGB565(a[7], a[8], a[9])*/);
            tft.print(getHWString(a[3]));
            break;
        }
      }
    } while (read != ';');
  }
  program.close();
  return 1;
}

String getHWString(int code) {
  String number;
  if (code == 0) {
    number = "000";
  } else if (code < 10) {
    number = "00" + code;
  } else if (code < 100) {
    number = "0" + code;
  } else {
    number = "" + code;
  }
  String path = "OS/STRINGS/" + number + ".txt";
  File string = SD.open(path);
  if (!string) {
    return "Failed";
  }
  String result;
  char read = '\0';
  char prev = '\0';
  while (string.available()) {
    read = string.read();
    if (read == 'n' && prev == '\\') {
      result[result.length() - 1] = '\0';
      result.concat('\n');
    } else {
      result.concat(read);
    }
    prev = read;
  }
  string.close();
  return result;
}

uint16_t getRGB565(int r, int g, int b) {
  return (b & 0b11111) | ((g & 0b111111) << 5) | ((r & 0b11111) << 11);
}



