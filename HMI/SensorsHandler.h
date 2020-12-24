#ifndef SENSORSHANDLER_H_DEFINED
#define SENSORSHANDLER_H_DEFINED

#include <Arduino.h>
#include <stdio.h>
#include <String.h>

#ifndef UINT8_MAX
#define UINT8_MAX 255
#endif
#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif

class SensorsHandler {
  private:
    uint8_t s0t = 0x00, s0h = 0x00;    // Sensor 0
    uint8_t s1t = 0x00, s1h = 0x00;    // Sensor 1
    uint8_t tmin = 0x19, tmax = 0x23;  // Temperature Range Min=25 Max=35
    uint8_t f0p = 0x00, f1p = 0x00;    // Fans power
    bool f0on = false, f1on = false;   // Fans switches
    uint16_t f0r = 0x00, f1r = 0x00;   // Fans rpms
    uint16_t m0u = 0x00, m0t = 0x8000; // 32768 bytes for Nano 33 IoT
    uint16_t m1u = 0x00, m1t = 0xA00;  // 2560 bytes for ProMicro ATmega32U4
    const uint8_t yesValue = 111;
    const uint8_t noValue = 222;
    uint8_t parseHex4(char value);
    uint8_t parseHex8(String value);
    uint16_t parseHex16(String value);
    
  public:
    // Constructors
    SensorsHandler();
    SensorsHandler(int mem0, int mem1);
    
    // Bulk input and output
    //bool fromUartMessage(char* message, size_t msgSize);
    bool fromUartMessage(String message);
    String debugString();
    
    //void setTotalMemory0(int value);  // Setted on initialization
    //void setTotalMemory1(int value);  // Setted on initialization
    void setFreeMemory0(int value) { m0u = value; }
    //void setUsedMemory1(int value);  // This value are updated from the uart message

    // Get the stored values
    uint8_t sensor0temperature() { return s0t; }
    uint8_t sensor0humidity() { return s0h; }
    uint8_t sensor1temperature() { return s1t; }
    uint8_t sensor1humidity() { return s1h; }
    uint8_t temperatureRangeMin() { return tmin; }
    uint8_t temperatureRangeMax() { return tmax; }
    uint8_t fan0power() { return f0p; }
    uint16_t fan0rpms() { return f0r; }
    bool fan0isOn() { return f0on; }
    uint8_t fan1power() { return f1p; }
    uint16_t fan1rpms() { return f1r; }
    bool fan1isOn() { return f1on; }
    uint16_t memory0free() { return m0u; }
    uint16_t memory1free() { return m1u; }
    uint16_t memory0total() { return m0t; }
    uint16_t memory1total() { return m1t; }
    
    // Get mqtt message payloads
    String sensor0message();
    String sensor1message();
    String fan0message();
    String fan1message();
    String rangeTmessage();
    String memory0message();
    String memory1message();
    

};

#endif
