#include <SMARTGPU2.h>
#include <Adafruit_FONA.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>

#define FONA_RX 2  // MEGA?
#define FONA_TX 3
#define FONA_RST -1
#define FONA_RI 6
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

SMARTGPU2 lcd;

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

byte openApp = 3;
byte screen = 0;

void setup() {
  wdt_reset();
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  
  /*lcd.init();
  lcd.start();
  lcd.baudChange(BAUD6);
  lcd.orientation(PORTRAIT_LOW);
  lcd.initDACAudio(ENABLE);
  lcd.audioBoost(ENABLE);
  lcd.bright(0);
  lcd.allocateVideoSD("start_anim", &metaVideo);
  lcd.bright(100);
  lcd.playWAVFile("start", 0);
  lcd.playVideoSD(0, 0, metaVideo.framesPerSec);*/
  
  /*fonaSerial->begin(4800);
  if (!fona.begin(*fonaSerial)) {
    lcd.imageBMPSD(0, 0, "start_anim_error");
    lcd.playWAVFile("error", 0);
    while (true) {}
  }*/

  Serial.begin(9600);
  Serial.println("Starting...");
  
  cli();
  wdt_reset();
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = (1<<WDIE) | (1<<WDE) | (0<<WDP3) | (1<<WDP2) | (1<<WDP1) | (0<<WDP0);
  sei();
}

void loop() {
  Serial.println("Bye");
  delay(500);
  //wdt_reset();
}

void draw(byte screen) {
  switch (screen) {
    case LOCK:
      lcd.imageBMPSD(0, 0, lock_wp);
      lcd.drawRoundRect(20, 50, 300, 50, 10, WHITE, (FILLGEOM)true);
      lcd.drawRoundRect(20, 50, 300, 50, 10, BLACK, (FILLGEOM)false);
      lcd.imageBMPSD(0, 261, "N_KP");
      break;
    case KP_C:
      while (1) {}
      break;
    case KP_N:
      while (1) {}
      break;
    case APP_SET:
      for (int i = 320; i >= 0; i--) {
        lcd.imageBMPSD(i, 0, "set_load");
        delay(1);
      }
      delay(1000);
      lcd.imageBMPSD(0, 0, "set_empty");
      lcd.imageBMPSD(0, 30, "set_pag1");
      break;
    case APP_PHONE:
      for (int i = 320; i >= 0; i--) {
        lcd.imageBMPSD(i, 0, "phone_load");
        delay(1);
      }
      delay(1000);
      lcd.imageBMPSD(0, 0, "app_phone");
      break;
    case APP_SMS:
      break;
    case APP_RADIO:
      break;
    default:
      while (1) {}
      break;
  }
}

void settings() {
  draw(APP_SET);
  while (true) {
    if (lcd.touchScreen(&tPoint) == VALID) {
      if (tPoint.x <= 30) {
        continue;
      }
    }
    wdt_reset();
  }
}

void call() {
  lcd.imageBMPSD(0, 0, "call1");
}

void call(char* number) {
  lcd.imageBMPSD(0, 0, "call2noMute");
}

ISR(WDT_vect) {
  digitalWrite(13, LOW);
  lcd.imageBMPSD(0, 0, "crash");
  lcd.playWAVFile("error", 0);
}

void checkAll() {
  if (digitalRead(FONA_RI) == 0) {
    call();
  }
}









