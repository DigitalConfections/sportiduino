#include <EEPROM.h>
#include <Adafruit_SleepyDog.h>
#include <string.h>
#include "sportiduino.h"

#define SERIAL_DATA_MAX_SIZE    28
#define SERIAL_TIMEOUT          10

void majEepromWrite(uint16_t adr, uint8_t val) {
    uint8_t oldVal = majEepromRead(adr);
    if(val != oldVal) {
        for(uint16_t i = 0; i < 3; i++) {
            EEPROM.write(adr + i, val);
        }
    }
}

uint8_t majEepromRead(uint16_t adr) {
    uint8_t val1 = EEPROM.read(adr);
    uint8_t val2 = EEPROM.read(adr + 1);
    uint8_t val3 = EEPROM.read(adr + 2);
    
    if(val1 == val2 || val1 == val3) {
        return val1;
    } else if(val2 == val3) {
        return val2;
    }
    
    //BEEP_EEPROM_ERROR;
    return 0;
}

void beep_w(const uint8_t ledPin, const uint8_t buzPin, uint16_t freq, uint16_t ms, uint8_t n) {
    for(uint8_t i = 0; i < n; i++) {
        Watchdog.reset();
        
        digitalWrite(ledPin, HIGH);
        if(freq > 0) {
            tone(buzPin, freq, ms);
        } else {
            digitalWrite(buzPin, HIGH);
        }
        
        delay(ms);
        Watchdog.reset();
        
        digitalWrite(ledPin, LOW);
        digitalWrite(buzPin, LOW);
        
        if(i < n - 1) {
            delay(ms);
            Watchdog.reset();
        }
    }
}

bool uint32ToByteArray(uint32_t value, byte *byteArray) {
    for(uint8_t i = 0; i < 4; ++i) {
        byteArray[3 - i] = value & 0xff;
        value >>= 8;
    }
    return true;
}

uint32_t byteArrayToUint32(byte *byteArray) {
    uint32_t value = 0;
    for(uint8_t i = 0; i < 4; ++i) {
        value <<= 8;
        value |= byteArray[i];
    }

    return value;
}

void SerialProtocol::init(uint8_t _startByte) {
    startByte = _startByte;
    Serial.setTimeout(SERIAL_TIMEOUT);
    begin();
}

void SerialProtocol::begin() {
    Serial.begin(9600);
}

void SerialProtocol::end() {
    Serial.end();
}

void SerialProtocol::start(uint8_t code) {
    serialDataPos = 3;
    serialPacketCount = 0;
    memset(serialBuffer, 0, SERIAL_PACKET_SIZE);

    serialBuffer[0] = startByte;
    serialBuffer[1] = code;
}

void SerialProtocol::send() {
    uint8_t dataSize = serialDataPos - 3; // minus start, resp code, datalen
    
    if(dataSize > SERIAL_DATA_MAX_SIZE) {
        dataSize = serialPacketCount + 0x1E;
        serialDataPos = SERIAL_PACKET_SIZE - 1;
        serialPacketCount++;
    }

    serialBuffer[2] = dataSize;
    serialBuffer[serialDataPos] = checkSum(serialBuffer, dataSize);

    for(uint8_t i = 0; i <= serialDataPos; i++) {
        Serial.write(serialBuffer[i]);
    }

    serialDataPos = 3;
}

void SerialProtocol::add(uint8_t dataByte) {
    if(serialDataPos >= SERIAL_PACKET_SIZE - 1) {
        serialDataPos++;  // to indicate that we going to send packet count
        send();
    }

    serialBuffer[serialDataPos] = dataByte;
    serialDataPos++;
}

void SerialProtocol::add(const uint8_t *data, uint8_t size) {
    for(uint8_t i = 0; i < size; ++i) {
        add(data[i]);
    }
}

uint8_t SerialProtocol::checkSum(uint8_t *buffer, uint8_t dataSize) {
    // if at dataSize position we have packet count
    if(dataSize > SERIAL_DATA_MAX_SIZE) {
        dataSize = SERIAL_DATA_MAX_SIZE;
    }

    uint8_t len = dataSize + 2;  // + cmd/resp byte + length byte
    uint8_t sum = 0;
    for (uint8_t i = 1; i <= len; ++i) {
        sum += buffer[i];
    }
    
    return sum;
}

uint8_t *SerialProtocol::read(bool *error, uint8_t *code, uint8_t *dataSize) {
    *error = false;
    memset(serialBuffer, 0, SERIAL_PACKET_SIZE);
    // Drop bytes before msg start
    while(Serial.available() > 0) {
        serialBuffer[0] = Serial.read();
        if(serialBuffer[0] == startByte) {
            Serial.readBytes(serialBuffer + 1, SERIAL_PACKET_SIZE);

            *dataSize = serialBuffer[2];

            // if at dataSize position we have packet count
            if(*dataSize > SERIAL_DATA_MAX_SIZE) {
                *dataSize = SERIAL_DATA_MAX_SIZE;  
            }

            if(serialBuffer[*dataSize + 3] != checkSum(serialBuffer, *dataSize)) {
                *error = true;
                return nullptr;
            }
            *code = serialBuffer[1];
            return &serialBuffer[3];
        }
    }
    return nullptr;
}

