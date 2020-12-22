#ifndef SENSORSHANDLER_H_DEFINED
#define SENSORSHANDLER_H_DEFINED

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

class SensorsHandler {
  private:
    uint8_t s0t = 0x00, s0h = 0x00;
    uint8_t s1t = 0x00, s1h = 0x00;
    uint8_t f0p = 0x00, f1p = 0x00;
    uint8_t tmin = 0x19, tmax = 0x23;  // Min=25 Max=35
    uint16_t m0u = 0x00, m0t = 0x8000; // 32768 bytes for Nano 33 IoT
    uint16_t m1u = 0x00, m1t = 0xA00;  // 2560 bytes for ProMicro ATmega32U4
    bool f0on = false, f1on = false;
    
  public:
    // Constructors
    SensorsHandler();
    SensorsHandler(int mem0, int mem1);
    
    // Bulk input and output
    bool fromUartMessage(char* message, size_t msgSize);
    char& toChars();
    
    void setTotalMemory0(int value);
    void setTotalMemory1(int value);
    void setUsedMemory0(int value);
    void setUsedMemory1(int value);
    
    // Get mqtt message payloads
    char& sensor0message();
    char& sensor1message();
    char& fan0message();
    char& fan1message();
    char& rangeTmessage();
    char& memory0message();
    char& memory1message();
    

};

#endif
