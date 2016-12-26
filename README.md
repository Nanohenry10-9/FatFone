# FatFone
The 2nd generation of [poDuino](https://github.com/Nanohenry10-9/poDuino).

Uses the [Adafruit 2.8" TFT Shield for Arduino w/ Capacitive Touch](https://www.adafruit.com/product/1947), [Adafruit Fona 800 Shield for Arduino](https://www.adafruit.com/product/2468) (with a loudspaker, microphone, a mini vibrator motor and a 1200mAh 3.7 volt battery), an [Arduino UNO](https://www.arduino.cc/en/Main/ArduinoBoardUno), a pushbutton and a RGB LED. NOTE: this hardware is only for FatFone 1. FatFOne 2 (FatOS 3 and up) require another hardware (described below).

FatFone is a simple mobile phone. It is called FatFone, because it is quite thick.


# FatOS 1

Ver. 1.50

Functions:
- Phone calls (calling, answering)
- Screen brightness and loudspeaker volume adjusting
- SMS sending

Future implementations (I already know how to make these, but there's only a small bit of flash memeory left):
- SMS receiving
- Pong game (already in the code, but commented out)
- Phone book
- Use RGB LED to display status (battery state, are there any new phone calls etc.)

To make the about-button (in the settings) work correctly, the text it will write on the screen has to be in the Arduino's built-in (or external, but it will require more physical space) EEPROM. This will free up the Arduino's flash memory a bit (or more than a bit (binary-digit)).


# FatOS 2

Ver. 2.8

Runs on FatFone 1 or FatFone 1M (same with FatOS 1). 

Functions:
- Phone calls (calling, answering)
- SMS receiving/reading and sending
- Volume Control

Future implementations:
- Settings app


# FatOS 3 (FatFone 2)

The most recent version of FatOS, which runs on FatFone 2.

Ver. pre-3.01

Required hardware:
- Arduino MEGA ADK
- Adafruit FONA 800 shield for Arduino (described at top of README)
- SmartGPU 2 320x480 pixel 3.5" display
- uSD card (and system files like graphics on the SD)
- RGB (status) LED
- VL6180X Adafruit Time of Flight sensor
- ADXL335 Adafruit Accelerometer


The logo:

![FatFone logo](FatFoneLogo.png)