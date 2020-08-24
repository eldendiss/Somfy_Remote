/* SomFy arduino library - based on nickduino's sketch
 * https://github.com/Nickduino/Somfy_Remote
 * Protocol details can be found here: https://pushstack.wordpress.com/somfy-rts-protocol/
 */
#pragma once

#ifndef SOMFY_H
#define SOMFY_H

enum Mov{
    Stop=1,     // Stops the movement/preffered positon
    My=1,       // Same as stop
    Up,         // Move up
    MyUp,       // Set upper motor limit in initial programming mode
    Down,       // Move down
    MyDown,     // Set lower motor limit in initial programming mode
    UpDown,     // Change motor limit and initial programming mode
    Prog=8,     // Used for (de-)registering remotes
    SunFlag,    // Enable sun and wind detector (SUN and FLAG symbol on the Telis Soliris RC)
    Flag        // Disable sun detector (FLAG symbol on the Telis Soliris RC)
};

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <EEPROM.h>
#define ADDRESS EEPROM.length()-2
#define SYMBOL 640

class SomFy
{
private:
    byte *prepPacket(uint8_t btn);
    uint8_t _pin=0;
    uint16_t _rollingC=0;
    uint16_t rollingCode=0;
    byte* payload;
    uint32_t _remoteAdd=0;
    void sendPacket(byte *_payload,bool first);
public:
    SomFy(uint8_t pin, uint32_t remoteAdd, uint16_t rollingC);
    int send(uint8_t btn,uint8_t cnt);
    int send(uint8_t cnt);
    int send(byte *packet, uint8_t cnt);
    void init();
};



#endif