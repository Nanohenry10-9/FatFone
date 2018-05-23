# FatFone

FatFone is an open source modular DIY Arduino-based mobile phone.

BTW, the name comes from the phone's thickness, which is about 3cm (still a prototype!).

All the parts can be easily detached and changed, so if a part breaks or gets technically outdated it can be easily fixed.

The prototype is built from three main parts:
- A GSM module, for example the Adafruit FONA 800
- A screen, like the 2.8" TFT LCD with touch screen from Adafruit
- An Arduino (UNO, Mega or whatever but Flash memory is a problem on smaller boards)

I programmed the whole current OS (FatOS) entirely by myself, except for the libraries, that I used for communicating with the different parts and work as a very basic "kernel" for my OS.

## FatFone and OS versions (briefly)

### FatFone 1

Arduino UNO as the main board.

#### OS version 1

The first OS version, but second graphical design. Originally it was more text-based, but the (newer) version 1.0 used graphical icons.

### FatFone 2

Not different from FatFone 1. Because why not (technically there **are** some changes).

#### OS version 2

The main difference to FatOS 1 is that the graphical design has completely changed.

### FatFone 2M & 2MB

The Arduino board has changed from UNO to Mega ADK, due to flash storage size limitations. The 2MB has two hardware buttons.

#### OS version 2.1

Basically a bigger and "better" version of FatOS 2.

#### OS version 2.2

A version of FatOS 2.1 but with the use of two external buttons on the FatFone 2MB (like Ardroid's home and back buttons).

#### OS version 2.3

The almost peak performance version of FatOS. Still not prefect though. It has a lot of cool functionalities.

#### OS version 2.4

The most recent OS. Basically, the whole thing just re-written from the base of version 2.3. Currently in "keyboard testbed -mode" (there is not much else than the keyboard). Also this version does not require the HW buttons so the FatFone version is just 2M (not 2MB like V2.2 & V2.3).

