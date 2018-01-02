# FatFone

FatFone is an open source modular DIY Arduino-based mobile phone.

BTW, the name comes from it's thickness, which is about 3cm (still a prototype!).

All the parts can be easily detached and changed, so if a part breaks or gets technically outdated it can be easily fixed.

The prototype is built from three main parts:
- A GSM module, for example the Adafruit FONA 800
- A screen, like the 2.8" TFT LCD with capacitive touch from Adafruit
- An Arduino (UNO, Mega or whatever but Flash memory is a problem on smaller boards)

I programmed the whole current OS (FatOS) 100% by myself, except for the libraries, that I used for communicating with the different parts and work as a kernel for my OS.

## FatFone and OS versions (briefly)

### FatFone 1

Arduino UNO as the main board.

#### OS version 1

The first OS version, but second graphical design. Originally it was more text-based, but the (newer) version 1.0 used graphical icons.

### FatFone 2

Not different from FatFone 1.

#### OS version 2

The main difference to FatOS 1 is that the graphical design has completely changed.

### FatFone 2M & 2MB

The Arduino has changed from UNO to Mega ADK, due to flash limitations.

#### OS version 2M

Basically a bigger version of FatOS 2

#### OS version 2MB

A version of FatOS 2M but with the use of two external buttons (like Ardroid's home and back buttons)

#### OS version 2.1MB

The most recent OS. It has a lot of functionslity already but is not working at the moment (possibly due to hardware getting old)

**This is where the OS versions begin being just attempts at doing something new*** 

#### OS version 3

An attemp at adding screen memory to the phone. Bad idea, as the Mega ADK only has 8Kb or RAM

#### OS version 4

An attemp at putting the OS code and all the apps on the SD card

#### OS version 5

Now, this version will be an actual version (unlike the previous two), with something that I do not really know either. And it will soon be under **heavy development**.