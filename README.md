# Avrdroid - Bluetooth based over the air programming  for atmel microcontroller

## File Description
* boot1.c         :  actual boot loader code
* avrdroid.pdf    :  Borad Schematics  file

## Compilation Steps 
1. Compiling bootloader : avr-gcc  -mmcu=atmega8 -Wall -Os boot1.c -N -Wl,--section-start=.text=0x1800,--section-start=.app=0x0040,--section-start=.myinit=0x0000
2. Converting to hex    : avr-objcopy -j .text -j .data -j .app -j .myinit -O ihex a.out boot1.hex

## Schematics 
![picture alt](https://github.com/mohit3112/AvrDroid/blob/master/hardware/bootloader/AVRDroid.png "AVRDROID BOOARD")
