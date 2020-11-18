/*   This sketch allows you to emulate a Somfy RTS or Simu HZ remote.
   If you want to learn more about the Somfy RTS protocol, check out https://pushstack.wordpress.com/somfy-rts-protocol/

   The rolling code will be stored in EEPROM, so that you can power the Arduino off.

   Easiest way to make it work for you:
    - Choose a remote number
    - Choose a starting point for the rolling code. Any unsigned int works, 1 is a good start
    - Upload the sketch
    - Long-press the program button of YOUR ACTUAL REMOTE until your blind goes up and down slightly
    - send 'p' to the serial terminal
  To make a group command, just repeat the last two steps with another blind (one by one)
*/



#include "SomFy.h"

#define PIN 5               //TX pin
#define REMOTE 0x121300     //Remote address<-- Change it!
#define ROLLING_CODE 101    //Rolling code initial value

SomFy blinds(PIN, REMOTE, ROLLING_CODE,
             &Serial); //(TX pin,Remote address,initial rolling code value, Serial port for debugging)
//SomFy blinds(PIN,REMOTE,101);         //(TX pin,Remote address,initial rolling code value)

void setup() {
    //Serial.begin(115200);     //this is not needed when debugging is enabled
    blinds.init();
}

void loop() {
    if (Serial.available() > 0) {
        char recv = (char)Serial.read();

        if (recv == 'u') {
            blinds.move(DIR_UP);    //move the blinds all the way up

        } else if (recv == 's') {
            blinds.stop();          //stop the movement

        } else if (recv == 'd') {
            blinds.move(DIR_DOWN);  //move the blinds all the way down

        } else if (recv == '+') {
            blinds.move(DIR_STEP_UP);  //Move blinds one step up

        } else if (recv == '-') {
            blinds.move(DIR_STEP_DOWN);  //Move blinds one step down

        } else if (recv == 'p') {
            blinds.send(C_PROG, 2); //send prog command, 2 repetitions

        } else {
            blinds.send(recv, 2);   //you can also send custom commands
        }
    }
}
