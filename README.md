# avrdroid
bluetooth based over the air programming  for atmel microcontroller<br />
////////////////////////////////////////////////////////////////////////////////////////////////////////////////<br />
boot1.c         ---------------->    actual boot loader code<br />
bootloader.dsn  ---------------->    proteus simulation file<br />
spm_test.S     ----------------->    a asm code to test my asm fuctions in boot1.c<br />
////////////////////////////////////////////////////////////////////////////////////////////////////////////////<br />


/////////////////////////////////////////  Compiling ///////////////////////////////////////////////////<br />

1.avr-gcc  -mmcu=atmega8 -Wall -Os boot1.c -N -Wl,--section-start=.text=0x1800,--section-start=.app=0x0040,--section-start=.myinit=0x0000<br />
2.avr-objcopy -j .text -j .data -j .app -j .myinit -O ihex a.out boot1.hex<br />


