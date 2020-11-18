#include "SomFy.h"

//TODO - test

/* Initialize class instance - prepare all variables
 * ARGS:  pin - 433.42MHz transmitter data pin,
 *          remoteAdd - simulated remote address,
 *          rollingC - Value to which will be rolling code initialized
 *                     (Only if its bigger than EEPROM value)
 */
SomFy::SomFy(uint8_t pin, const uint32_t remoteAdd, uint16_t rollingC) {
    _remoteAdd = remoteAdd;
    _pin = pin;
    _rollingC = rollingC;
    debug = 0;
}


/* Initialize class instance - prepare all variables
 * ARGS:  pin - 433.42MHz transmitter data pin,
 *          remoteAdd - simulated remote address,
 *          rollingC - Value to which will be rolling code initialized
 *                     (Only if its bigger than EEPROM value)
 *          *_serial - HW/SW serial for debug prints
 */
SomFy::SomFy(uint8_t pin, const uint32_t remoteAdd, uint16_t rollingC, HardwareSerial *serial) {
    _remoteAdd = remoteAdd;
    _pin = pin;
    _rollingC = rollingC;
    _serial = serial;
    debug = 1;
}


/* Init - Hardware configuration
 * ARGS: void
 * RETURNS: void
 */
void SomFy::init() {
    pinMode(_pin, 0x1);      //pin as output
    digitalWrite(_pin, 0);   //set as low

    if (EEPROM.get(ADDRESS, rollingCode) < _rollingC) {
        EEPROM.put(ADDRESS, _rollingC);
        rollingCode = _rollingC;
    }

    if (debug) {
        _serial->begin(38400);
        _serial->print("Simulated remote number : ");
        _serial->println(_remoteAdd, HEX);
        _serial->print("Current rolling code    : ");
        _serial->println(rollingCode);
    }
}


/* prepPacket - prepare packet bytes, private
 * ARGS: uint8_t btn - button code
 * RETURNS: byte* prepared payload;
 */
byte *SomFy::prepPacket(uint8_t btn) {
    if ((payload = (byte *)malloc(7)) == NULL) {
        return NULL;
    }

    EEPROM.get(ADDRESS, rollingCode);

    payload[0] = 0xA7;                  // Encryption key. Doesn't matter much
    payload[1] = btn << 4;              // Which button did  you press? The 4 LSB will be the checksum
    payload[2] = rollingCode >> 8;      // Rolling code (big endian)
    payload[3] = rollingCode;           // Rolling code
    payload[4] = _remoteAdd >> 16;      // Remote address
    payload[5] = _remoteAdd >>  8;      // Remote address
    payload[6] = _remoteAdd;            // Remote address

    byte checksum = 0;

    for (byte i = 0; i < 7; i++) {  //checksum calculation
        checksum = checksum ^ payload[i] ^ (payload[i] >> 4);
    }

    checksum &= 0b1111;             // We are only interested in 4 LSB bits
    payload[1] |= checksum;         // Append checksum as 4 LSB in 2nd payload frame

    for (byte i = 1; i < 7; i++) {  // Payload obfuscation - XOR of all bytes
        payload[i] ^= payload[i - 1];
    }

    EEPROM.put(ADDRESS, ++rollingCode);

    if (debug) {
        _serial->print("Payload         : ");

        for (byte i = 0; i < 7; i++) {
            if (payload[i] >> 4 == 0) { //  Displays leading zero in case the most significant
                _serial->print("0");     // nibble is a 0.
            }

            _serial->print(payload[i], HEX);
            _serial->print(" ");
        }

        _serial->println("");
        _serial->print("With checksum : ");

        for (byte i = 0; i < 7; i++) {
            if (payload[i] >> 4 == 0) {
                _serial->print("0");
            }

            _serial->print(payload[i], HEX);
            _serial->print(" ");
        }

        _serial->println("");
        _serial->print("Obfuscated    : ");

        for (byte i = 0; i < 7; i++) {
            if (payload[i] >> 4 == 0) {
                _serial->print("0");
            }

            _serial->print(payload[i], HEX);
            _serial->print(" ");
        }

        _serial->println("");
        _serial->print("Rolling Code  : ");
        _serial->println(rollingCode);
    }

    return payload;
}

/* sendPacket - private function - sending sequence
 * ARGS: byte *_payload - prepared packet, bool first - is this 1st packet? for HW sync.
 * RETURNS: void
 */
void SomFy::sendPacket(byte *_payload, bool first) {
    /** Wakeup pulse & silence **/
    digitalWrite(_pin, 1);
    delayMicroseconds(9415);
    digitalWrite(_pin, 0);
    delayMicroseconds(89565);
    //delay(90);    //delayMicroseconds is not accurate if delay > 16383, lets try delay instead

    /** hardware sync **/
    for (int i = 0; i < (first ? 2 : 7); i++) {
        digitalWrite(_pin, 1);
        delayMicroseconds(4 * SYMBOL);
        digitalWrite(_pin, 0);
        delayMicroseconds(4 * SYMBOL);
    }

    /** Software sync **/
    digitalWrite(_pin, 1);
    delayMicroseconds(4550);
    digitalWrite(_pin, 0);
    delayMicroseconds(SYMBOL);

    /** Data **/
    for (byte i = 0; i < 56; i++) {
        if (((_payload[i / 8] >> (7 - (i % 8))) & 1) == 1) {
            digitalWrite(_pin, 0);
            delayMicroseconds(SYMBOL);
            digitalWrite(_pin, 1);
            delayMicroseconds(SYMBOL);

        } else {
            digitalWrite(_pin, 1);
            delayMicroseconds(SYMBOL);
            digitalWrite(_pin, 0);
            delayMicroseconds(SYMBOL);
        }
    }

    /** Silence **/
    digitalWrite(_pin, 0);
    delayMicroseconds(30415);
    //delay(31);    //delayMicroseconds is not accurate if us > 16383, lets try delay instead
}


/* send - send packet - prepared with prepPacket
 * ARGS: uint8_t cnt - # of repetitions
 * RETURNS: nonzero if succesful, zero if fails
 */
/*int SomFy::send(uint8_t cnt) {
    if(payload==NULL||cnt<0)
        return 0;

    sendPacket(payload,true);

    for(byte i=0;i<cnt;i++){
        sendPacket(payload,false);
    }
    free(payload);      //release memory - not needed anymore
    return 1;
}*/


/* send - send and build packet
 * ARGS: uint8_t btn - button command, uint8_t cnt - # of repetitions
 * RETURNS: nonzero if succesful, zero if fails
 */
int SomFy::send(uint8_t btn, uint8_t cnt) {
    if (cnt < 0 || prepPacket(btn) == NULL) {
        return 0;
    }

    sendPacket(payload, true);

    for (byte i = 0; i < cnt; i++) {
        sendPacket(payload, false);
    }

    free(payload);      //release memory - not needed anymore
    return 1;
}


/* send - send prepared packet with repetition option
 * you can call both functions in one line: send(prepPacket(btn),2);
 * ARGS: byte *packet - prepared packet, uint8_t cnt - # of repetitions
 * RETURNS: nonzero if succesful, zero if fails
 */
int SomFy::send(byte *packet, uint8_t cnt) {
    if (packet == NULL) {
        return 0;
    }

    sendPacket(packet, true);

    for (byte i = 0; i < cnt; i++) {
        sendPacket(packet, false);
    }

    free(payload);      //release memory - not needed anymore
    return 1;
}


/* move - move blinds in provided direction
 * ARGS: uint8_t _dir - direction from enum dir
 * RETURNS: void
 */
void SomFy::move(dir_t _dir) {
    switch (_dir) {
        case DIR_UP:
            if (debug) {
                _serial->println("UP");
            }

            send(prepPacket(C_UP), 16);
            break;

        case DIR_STEP_UP:
            if (debug) {
                _serial->println("STEP UP");
            }

            send(prepPacket(C_UP), 2);
            break;

        case DIR_STEP_DOWN:
            if (debug) {
                _serial->println("STEP DOWN");
            }

            send(prepPacket(C_DOWN), 2);
            break;

        case DIR_DOWN:
            if (debug) {
                _serial->println("DOWN");
            }

            send(prepPacket(C_DOWN), 16);
            break;

        case DIR_STOP:
            if (debug) {
                _serial->println("STOP");
            }

            send(prepPacket(C_STOP), 2);
            break;

        default:
            return;
    }
}


/* stop - stops the movement
 * ARGS: void
 * RETURNS: void
 */
void SomFy::stop() {
    _serial->println("STOP");
    send(prepPacket(C_STOP), 2);
}