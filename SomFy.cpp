#include "SomFy.h"

//TODO - test

/* Initialize class instance - prepare all variables 
 * PARAMS:  pin - 433.42MHz transmitter data pin, 
 *          remoteAdd - simulated remote address,
 *          rollingC - Value to which will be rolling code initialized
 *                     (Only if its bigger than EEPROM value)
 * RETURNS: void
 */
SomFy::SomFy(uint8_t pin, const uint32_t remoteAdd, uint16_t rollingC) {
    _remoteAdd=remoteAdd;
    _pin=pin;
    _rollingC=rollingC;
}

/* Init - Hardware configuration
 * PARAMS: void
 * RETURNS: void
 */
void SomFy::init(){
    if((payload=(byte*)malloc(7))==NULL){
        return;
    }
    
    pinMode(_pin,0x1);       //pin as output
    digitalWrite(_pin,0);    //set as low
     if(EEPROM.get(ADDRESS,rollingCode)<_rollingC)
        EEPROM.put(ADDRESS, _rollingC);
}


/* prepPacket - prepare packet bytes
 * PARAMS: uint8_t btn - button code
 * RETURNS: prepared payload;
 */
byte* SomFy::prepPacket(uint8_t btn){
    if(payload==NULL)
        return NULL;
    
    EEPROM.get(ADDRESS,rollingCode);

    payload[0] = 0xA7;                  // Encryption key. Doesn't matter much
    payload[1] = btn << 4;              // Which button did  you press? The 4 LSB will be the checksum
    payload[2] = rollingCode >> 8;      // Rolling code (big endian)
    payload[3] = rollingCode;           // Rolling code
    payload[4] = _remoteAdd >> 16;      // Remote address
    payload[5] = _remoteAdd >>  8;      // Remote address
    payload[6] = _remoteAdd;            // Remote address

    byte checksum = 0;
    for(byte i = 0; i < 7; i++) {   //checksum calculation
        checksum = checksum ^ payload[i] ^ (payload[i] >> 4);
    }
    checksum &= 0b1111;             // We are only interested in 4 LSB bits
    payload[1] |= checksum;         // Append checksum as 4 LSB in 2nd payload frame

    for(byte i = 1; i < 7; i++) {   // Payload obfuscation - XOR of all bytes
        payload[i] ^= payload[i-1];
    }

    EEPROM.put(ADDRESS, ++rollingCode);
    return payload;
}


void SomFy::sendPacket(byte *_payload,bool first){
    /** Wakeup pulse & silence **/
    digitalWrite(_pin,1);
    delayMicroseconds(9415);
    digitalWrite(_pin,0);
    delayMicroseconds(89565);

    /** hardware sync **/
    for (int i = 0; (first?2:7); i++) {   
        digitalWrite(_pin,1);
        delayMicroseconds(4*SYMBOL);
        digitalWrite(_pin,0);
        delayMicroseconds(4*SYMBOL);
    }

    /** Software sync **/
    digitalWrite(_pin,1);
    delayMicroseconds(4550);
    digitalWrite(_pin,0);
    delayMicroseconds(SYMBOL);

    /** Data **/
    for(byte i = 0; i < 56; i++) {
        if(((_payload[i/8] >> (7 - (i%8))) & 1) == 1) {
            digitalWrite(_pin,0);
            delayMicroseconds(SYMBOL);
            digitalWrite(_pin,1);
            delayMicroseconds(SYMBOL);
        } else {
            digitalWrite(_pin,1);
            delayMicroseconds(SYMBOL);
            digitalWrite(_pin,0);
            delayMicroseconds(SYMBOL);
        }
    }

    /** Silence **/
    digitalWrite(_pin,0);
    delayMicroseconds(30415);
}


/* send - send packet - prepared with prepPacket
 * PARAMS: uint8_t cnt - # of repetitions
 * RETURNS: nonzero if succesful, zero if fails
 */
int SomFy::send(uint8_t cnt) {
    if(payload==NULL)
        return 0;

    sendPacket(payload,true);

    for(byte i=0;i<cnt;i++){
        sendPacket(payload,false);
    }
    return 1;
}


/* send - send and build packet
 * PARAMS: uint8_t btn - button command, uint8_t cnt - # of repetitions
 * RETURNS: nonzero if succesful, zero if fails
 */
int SomFy::send(uint8_t btn,uint8_t cnt) {
    if(prepPacket(btn)==NULL)
        return 0;

    sendPacket(payload,true);

    for(byte i=0;i<cnt;i++){
        sendPacket(payload,false);
    }
    return 1;
}


/* send - send and build packet
 * PARAMS: byte *packet - prepared packet, uint8_t cnt - # of repetitions
 * RETURNS: nonzero if succesful, zero if fails
 */
int SomFy::send(byte *packet, uint8_t cnt) {
    if(packet==NULL){
        return 0;
    }

    sendPacket(packet,true);

    for(byte i=0;i<cnt;i++){
        sendPacket(packet,false);
    }
}