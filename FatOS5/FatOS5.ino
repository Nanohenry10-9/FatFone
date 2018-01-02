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

void setup() {
  fonaSerial->begin(4800);
  fona.begin();

}

void loop() {
  // put your main code here, to run repeatedly:

}
