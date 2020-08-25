![Image of the license](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)

# Somfy Remote
An arduino library for emulating SomFy RTS protocol. Based on https://github.com/Nickduino/Somfy_Remote arduino sketch.


If you want to learn more about the Somfy RTS protocol, check out [Pushtack](https://pushstack.wordpress.com/somfy-rts-protocol/).



**How the hardware works:**
Connect a *433.42 Mhz* RF transmitter to any arduino pin. If you can't find 433.*42*Mhz transmitter, it's possible to change the SAW resonator in any 433.xx Mhz transmitter. Just verify that it supports ASK/OOK modulation. If the range sucks, try upgrading antenna to 1/4 wave length (simple ~17cm straight 30AWG wire will do).


**How the software works:**

The rolling code value is stored in the EEPROM (at the very end), so that count won't be lost during reset/power down.

*SomFy classname (PIN, REMOTE ADDRESS, ROLLING CODE, SERIAL PORT)*\
**PIN** - Arduino pin, to which transmitter is connected.\
**REMOTE ADDRESS** - Transmitter address to emulate\
**ROLLING CODE** - Initial value of rolling code\
**SERIAL PORT** - HardwareSerial* - Optional, library prints out debug data to provided serial port.

*init()* - pin configuration, eeprom rolling code reading and opening serial port

*send(BUTTON, COUNT)* - builds packet from provided BUTTON and sends it COUNT times\
**BUTTON** - which buttons to emulate. Predefined values are: C_STOP, C_MY, C_UP, C_MYUP, C_DOWN, C_MYDOWN, C_UPDOWN, C_PROG, C_SUNFLAG, C_FLAG. But you can use any custom value up to 0xFF\
**COUNT** - how many times will be the command sent

*send(PACKET, COUNT)* - send provided PACKET COUNT times.\
**PACKET** - custom packet. Data type is byte*, maximum array length is 7.\
**COUNT** - how many times will be the command sent

*move(DIRECTION)* - move blinds in provided direction\
**DIRECTION** - where to move: DIR_UP, DIR_DOWN, DIR_STEP_UP, DIR_STEP_DOWN and also DIR_STOP;

*stop()* - stops the movement