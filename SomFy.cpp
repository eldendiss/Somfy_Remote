#include "SomFy.h"

//TODO - test

SomFy::SomFy(uint8_t pin, uint32_t remoteAdd, uint16_t rollingC):
    _pin(pin),
    _remoteAdd(remoteAdd),
    _rollingCodeDefault(rollingC),
    _serial(NULL) {
    pinMode(_pin, OUTPUT);   // Pin as output
    digitalWrite(_pin, 0);   // Set as low
}

SomFy::SomFy(uint8_t pin, uint32_t remoteAdd, uint16_t rollingC, HardwareSerial *serial, uint32_t baud):
    _pin(pin),
    _remoteAdd(remoteAdd),
    _rollingCodeDefault(rollingC),
    _serial(serial),
    _baud(baud) {
    pinMode(_pin, OUTPUT);   // Pin as output
    digitalWrite(_pin, 0);   // Set as low
}

void SomFy::init() {
    if (EEPROM.get(SOMFY_ADDRESS, _rollingCode) < _rollingCodeDefault) {
        EEPROM.put(SOMFY_ADDRESS, _rollingCodeDefault);
        _rollingCode = _rollingCodeDefault;
    }

    if (_serial) {
        _serial->begin(_baud);
        _serial->print("Simulated remote number : ");
        _serial->println(_remoteAdd, HEX);
        _serial->print("Current rolling code    : ");
        _serial->println(_rollingCode);
    }
}

byte *SomFy::prepPacket(uint8_t btn) {
    if ((payload = (byte *)malloc(7)) == NULL) {
        return NULL;
    }

    EEPROM.get(SOMFY_ADDRESS, _rollingCode);

    payload[0] = 0xA7;                  // Encryption key. Doesn't matter much
    payload[1] = btn << 4;              // Which button did  you press? The 4 LSB will be the checksum
    payload[2] = _rollingCode >> 8;      // Rolling code (big endian)
    payload[3] = _rollingCode;           // Rolling code
    payload[4] = _remoteAdd >> 16;      // Remote address
    payload[5] = _remoteAdd >>  8;      // Remote address
    payload[6] = _remoteAdd;            // Remote address

    byte checksum = 0;

    for (byte i = 0; i < 7; i++) {  // Checksum calculation
        checksum = checksum ^ payload[i] ^ (payload[i] >> 4);
    }

    checksum &= 0b1111;             // We are only interested in 4 LSB bits
    payload[1] |= checksum;         // Append checksum as 4 LSB in 2nd payload frame

    for (byte i = 1; i < 7; i++) {  // Payload obfuscation - XOR of all bytes
        payload[i] ^= payload[i - 1];
    }

    EEPROM.put(SOMFY_ADDRESS, _rollingCode++);

    if (_serial) {
        _serial->print("Payload         : ");

        for (byte i = 0; i < 7; i++) {
            if (payload[i] >> 4 == 0) {  // Displays leading zero in case the most significant
                _serial->print("0");     // Nibble is a 0.
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
        _serial->println(_rollingCode);
    }

    return payload;
}

void SomFy::sendPacket(byte *_payload, bool first) {
    /** Wakeup pulse & silence **/
    digitalWrite(_pin, 1);
    delayMicroseconds(9415);
    digitalWrite(_pin, 0);
    delayMicroseconds(89565);
    // delay(90);    // delayMicroseconds is not accurate if delay > 16383, lets try delay instead

    /** hardware sync **/
    for (byte i = 0; i < (first ? 2 : 7); i++) {
        digitalWrite(_pin, 1);
        delayMicroseconds(4 * SOMFY_SYMBOL);
        digitalWrite(_pin, 0);
        delayMicroseconds(4 * SOMFY_SYMBOL);
    }

    /** Software sync **/
    digitalWrite(_pin, 1);
    delayMicroseconds(4550);
    digitalWrite(_pin, 0);
    delayMicroseconds(SOMFY_SYMBOL);

    /** Data **/
    for (byte i = 0; i < 56; i++) {
        if (((_payload[i / 8] >> (7 - (i % 8))) & 1) == 1) {
            digitalWrite(_pin, 0);
            delayMicroseconds(SOMFY_SYMBOL);
            digitalWrite(_pin, 1);
            delayMicroseconds(SOMFY_SYMBOL);

        } else {
            digitalWrite(_pin, 1);
            delayMicroseconds(SOMFY_SYMBOL);
            digitalWrite(_pin, 0);
            delayMicroseconds(SOMFY_SYMBOL);
        }
    }

    /** Silence **/
    digitalWrite(_pin, 0);
    delayMicroseconds(30415);
    // delay(31);    // delayMicroseconds is not accurate if us > 16383, lets try delay instead
}

/*int SomFy::send(uint8_t cnt) {
    if(payload==NULL||cnt<0)
        return 0;

    sendPacket(payload,true);

    for(byte i=0;i<cnt;i++){
        sendPacket(payload,false);
    }
    free(payload);      // Release memory - not needed anymore
    return 1;
}*/

int SomFy::send(uint8_t btn, uint8_t cnt) {
    if (cnt < 0 || prepPacket(btn) == NULL) {
        return 0;
    }

    sendPacket(payload, true);

    for (byte i = 0; i < cnt; i++) {
        sendPacket(payload, false);
    }

    free(payload); // Release memory - not needed anymore
    return 1;
}

int SomFy::send(byte *packet, uint8_t cnt) {
    if (packet == NULL) {
        return 0;
    }

    sendPacket(packet, true);

    for (byte i = 0; i < cnt; i++) {
        sendPacket(packet, false);
    }

    free(payload); // Release memory - not needed anymore
    return 1;
}

void SomFy::move(dir_t _dir) {
    switch (_dir) {
        case DIR_UP:
            if (_serial) {
                _serial->println("UP");
            }

            send(prepPacket(C_UP), 16);
            break;

        case DIR_STEP_UP:
            if (_serial) {
                _serial->println("STEP UP");
            }

            send(prepPacket(C_UP), 2);
            break;

        case DIR_STEP_DOWN:
            if (_serial) {
                _serial->println("STEP DOWN");
            }

            send(prepPacket(C_DOWN), 2);
            break;

        case DIR_DOWN:
            if (_serial) {
                _serial->println("DOWN");
            }

            send(prepPacket(C_DOWN), 16);
            break;

        case DIR_STOP:
            stop();
            break;

        default:
            return;
    }
}

void SomFy::stop() {
    if (_serial) {
        _serial->println("STOP");
    }

    send(prepPacket(C_STOP), 2);
}