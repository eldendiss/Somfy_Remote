/* SomFy arduino library - based on nickduino's sketch
 * https://github.com/Nickduino/Somfy_Remote
 * Protocol details can be found here: https://pushstack.wordpress.com/somfy-rts-protocol/
 */
#pragma once

#ifndef SOMFY_H
#define SOMFY_H

typedef enum {
    C_STOP = 1,   // Stops the movement/preffered positon
    C_MY = 1,     // Same as stop
    C_UP,         // Move up
    C_MYUP,       // Set upper motor limit in initial programming mode
    C_DOWN,       // Move down
    C_MYDOWN,     // Set lower motor limit in initial programming mode
    C_UPDOWN,     // Change motor limit and initial programming mode
    C_PROG = 8,   // Used for (de-)registering remotes
    C_SUNFLAG,    // Enable sun and wind detector (SUN and FLAG symbol on the Telis Soliris RC)
    C_FLAG        // Disable sun detector (FLAG symbol on the Telis Soliris RC)
} cmd_t;

typedef enum {
    DIR_UP = 0,     // All the way up
    DIR_STEP_UP,    // One step up
    DIR_STEP_DOWN,  // One step down
    DIR_DOWN,       // All the way down
    DIR_STOP        // Stops the movement
} dir_t;

#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

#include <EEPROM.h>
#define SOMFY_ADDRESS EEPROM.length()-2
#define SOMFY_SYMBOL 640

class SomFy {
  private:
    /**
     * @brief prepare packet bytes
     *
     * @param btn button code
     * @return prepared payload
     */
    byte *prepPacket(uint8_t btn);

    /**
     * @brief Sending sequence
     *
     * @param _payload prepared packet
     * @param first is this 1st packet? for HW sync
     */
    void sendPacket(byte *_payload, bool first);

    HardwareSerial *_serial;
    bool debug = 0;
    uint8_t _pin = 0;
    uint16_t _rollingC = 0;
    uint16_t rollingCode = 0;
    uint32_t _remoteAdd = 0;
    byte *payload;

  public:
    /**
     * @brief Construct a new SomFy object
     * 
     * @param pin 433.42MHz transmitter data pin
     * @param remoteAdd simulated remote address
     * @param rollingC value to which will be rolling code initialized
     * (Only if its bigger than EEPROM value)
     */
    SomFy(uint8_t pin, uint32_t remoteAdd, uint16_t rollingC);

    /**
     * @brief Construct a new SomFy object
     *
     * @param pin 433.42MHz transmitter data pin
     * @param remoteAdd simulated remote address
     * @param rollingC value to which will be rolling code initialized
     * (Only if its bigger than EEPROM value)
     * @param _serial HW/SW serial for debug prints
     */
    SomFy(uint8_t pin, uint32_t remoteAdd, uint16_t rollingC, HardwareSerial *_serial);

    /**
     * @brief Hardware configuration
     *
     */
    void init();

    /**
     * @brief Send and build packet
     *
     * @param btn button command
     * @param cnt # of repetitions
     * @return nonzero if succesful, zero if fails
     */
    int send(uint8_t btn, uint8_t cnt);

    // int send(uint8_t cnt);

    /**
     * @brief Send prepared packet with repetition option
     *
     * @param packet prepared packet
     * @param cnt # of repetitions
     * @return nonzero if succesful, zero if fails
     *
     * @note you can call both functions in one line: send(prepPacket(btn),2);
     */
    int send(byte *packet, uint8_t cnt);

    /**
     * @brief Move blinds in provided direction
     *
     * @param _dir direction from enum dir
     */
    void move(dir_t _dir);

    /**
     * @brief stops the movement
     *
     */
    void stop();
};

#endif