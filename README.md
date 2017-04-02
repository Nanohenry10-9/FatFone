# FatFone

FatFone is an open source modular DIY Arduino-based mobile phone.

All the parts can be asily detached and chamged, so if a part breaks or gets technically outdated it can be easily fixed.

It is built from three main parts:
- A GSM module, for that I am currently using an Adafruit FONA 800
- A screen, for that I am using a 2.8" TFT LCD with capacitive touch
- An Arduino, that controls the whole phone

I programmed the whole OS (about 2000 lines) 100% by myself, except for the libraries, that I used for communicating with the different parts.


## About FatFone

The 2nd generation of [poDuino](https://github.com/Nanohenry10-9/poDuino).

Uses the [Adafruit 2.8" TFT Shield for Arduino w/ Capacitive Touch](https://www.adafruit.com/product/1947), [Adafruit Fona 800 Shield for Arduino](https://www.adafruit.com/product/2468) (with a loudspaker, microphone, a mini vibrator motor and a 1200mAh 3.7 volt battery), an [Arduino UNO](https://www.arduino.cc/en/Main/ArduinoBoardUno), a pushbutton and a RGB LED. NOTE: this hardware is only for FatFone 1. FatFOne 2 (FatOS 3 and up) require another hardware (described below).

FatFone is a simple mobile phone. It is called FatFone, because it is quite thick.

There are different versions of FatFone and FatOS (the operating system).


### FatFone & FatOS 1

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


### FatFone & FatOS 2

Runs on FatFone 1 or FatFone 1M (same with FatOS 1). 

Functions:
- Phone calls (calling, answering)
- SMS receiving/reading and sending
- Volume Control

Future implementations:
- Settings app

### FatFone & FatOS 2M & 2MB

Runs on FatFone 2M and 2MB

This is an updated version of FatOS 2 with more flash usage and two extenal buttons (only FatFone & FatOS 2MB) (home and back, pins 18 and 19 in that order).

All the FatOS 2 functions are still there, but there is also some added functions such as a paint app, radio app etc.

Hardware:
- Arduino Mega ADK
- Arduino compatible Adafruit FONA 800 shield and a loudspeaker, mic and other required hardware (described at the top)
- Adafruit 2.8" TFT shield for Arduino w/ Capacitive Touch
- Two pushbuttons and jumper wires


### FatFone & FatOS 3

The newest FatOS/FatFone. Hardware is still under heavy developement, and is not listed below.

This version will not be using an Arduino board and shields. I will instead make a PCB and buy non-shield versions of the shields that I am currently using. I wil also use a spare ATMega chip, like the ATMega328.


The logo:

![FatFone logo](FatFoneLogo.png)