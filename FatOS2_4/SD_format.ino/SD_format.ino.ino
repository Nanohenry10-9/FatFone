#include <SD.h>

void setup() {
  Serial.begin(115200);
  SD.begin(4);
  File root = SD.open("/");
  root.rewindDirectory();
  while (1) {
    File entry = root.openNextFile();
    if (!entry)
      break;
    Serial.println(entry.name());      
    SD.remove(entry.name());
  }
  File entry = SD.open("SETTING.TXT", FILE_WRITE);
  entry.write(128);
  entry.write(50);
  entry.write(1);
  entry.close();
  Serial.println("Verifying...");
  entry = SD.open("SETTING.TXT", FILE_READ);
  Serial.println(entry.read(), DEC);
  Serial.println(entry.read(), DEC);
  Serial.println(entry.read(), DEC);
  entry.close();
  Serial.println("OK!");
}

void loop() {}
