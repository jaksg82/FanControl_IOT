#include "SensorsHandler.h"

//----------------------------------------------------------------
// Constructor
//----------------------------------------------------------------
SensorsHandler::SensorsHandler(){}

SensorsHandler::SensorsHandler(int mem0 = 32768, int mem1 = 2560){
    // Initialize
    m0t = mem0; // Nano 33 IoT
    m1t = mem1; // ProMicro ATmega32U4
}

//----------------------------------------------------------------
// Private functions
//----------------------------------------------------------------
uint8_t SensorsHandler::parseHex4(char value) {
  switch (value) {
    case '0': return 0; break;
    case '1': return 1; break;
    case '2': return 2; break;
    case '3': return 3; break;
    case '4': return 4; break;
    case '5': return 5; break;
    case '6': return 6; break;
    case '7': return 7; break;
    case '8': return 8; break;
    case '9': return 9; break;
    case 'A': return 10; break;
    case 'a': return 10; break;
    case 'B': return 11; break;
    case 'b': return 11; break;
    case 'C': return 12; break;
    case 'c': return 12; break;
    case 'D': return 13; break;
    case 'd': return 13; break;
    case 'E': return 14; break;
    case 'e': return 14; break;
    case 'F': return 15; break;
    case 'f': return 15; break;
    case 'Y': return yesValue; break;
    case 'N': return noValue; break;
    default: return 255;
  }
}

uint8_t SensorsHandler::parseHex8(String value) {
  uint8_t retVal = UINT8_MAX;
  if (value.length() >= 2) {
     uint8_t c0 = parseHex4((char)value.charAt(0));
     uint8_t c1 = parseHex4((char)value.charAt(1));
     if (c0 != 255 && c1 != 255) {
      retVal = (c0 * 16) + c1;
     }
  }
  return retVal;
}

uint16_t SensorsHandler::parseHex16(String value) {
  uint16_t retVal = UINT16_MAX;
  if (value.length() >= 4) {
     uint8_t c0 = parseHex4((char)value.charAt(0));
     uint8_t c1 = parseHex4((char)value.charAt(1));
     uint8_t c2 = parseHex4((char)value.charAt(2));
     uint8_t c3 = parseHex4((char)value.charAt(3));
     if (c0 != 255 && c1 != 255 && c2 != 255 && c3 != 255) {
      retVal = (c0 * 4096) + (c1 * 256) + (c2 * 16) + c3;
     }
  }
  return retVal;
}


//----------------------------------------------------------------
// Public input functions
//----------------------------------------------------------------


bool SensorsHandler::fromUartMessage(String message){
  // Make some chacks on the given message string
  if (message.length() < 35) { return false; } // String too short
  if (message.charAt(0) != '$') { return false; } // String do not start with the correct char
  if (message.charAt(5) != ',') { return false; } // String delimiter not in the correct position
  if (message.charAt(34) != ';') { return false; } // String do not end with the correct char

  // Parse the values
  uint8_t v0 = parseHex8(message.substring(6, 8));
  uint8_t v1 = parseHex8(message.substring(8, 10));
  uint8_t v2 = parseHex8(message.substring(10, 12));
  uint8_t v3 = parseHex8(message.substring(12, 14));
  uint8_t v4 = parseHex8(message.substring(14, 16));
  uint8_t v5 = parseHex8(message.substring(16, 18));
  uint8_t v6 = parseHex8(message.substring(18, 20));
  uint8_t v7 = parseHex4((char)message.charAt(20));
  uint16_t v8 = parseHex16(message.substring(21, 25));
  uint8_t v9 = parseHex4((char)message.charAt(25));
  uint16_t v10 = parseHex16(message.substring(26, 30));
  uint16_t v11 = parseHex16(message.substring(30, 34));

  // Check and store the values
  if (v0 != UINT8_MAX) { s0t = v0; }
  if (v1 != UINT8_MAX) { s0h = v1; }
  if (v2 != UINT8_MAX) { s1t = v2; }
  if (v3 != UINT8_MAX) { s1h = v3; }
  if (v4 != UINT8_MAX) { tmin = v4; }
  if (v5 != UINT8_MAX) { tmax = v5; }
  if (v6 != UINT8_MAX) { f0p = v6; f1p = f0p; }  // There is only one power value for both fans
  if (v7 != UINT8_MAX) { f0on = (v7 == yesValue); }
  if (v8 != UINT16_MAX) { f0r = v8; }
  if (v9 != UINT8_MAX) { f1on = (v9 == yesValue); }
  if (v10 != UINT16_MAX) { f1r = v10; }
  if (v11 != UINT16_MAX) { m1u = v11; }
  return true;
}

bool SensorsHandler::fromUartMessage(String message, uint32_t epoch){
  msgEpoch = epoch;
  fromUartMessage(message);
  return true;
}

//----------------------------------------------------------------
// Public output functions
//----------------------------------------------------------------

String SensorsHandler::debugString() {
  // Add Sensors
  String retMsg = "Sensors:" + String(s0t) + "|" + String(s0h) + "|" + String(s1t) + "|" + String(s1h) + "|" + String(tmin) + "|" + String(tmax);
  // Add Fans
  retMsg += " Fans:" + String(f0p) + "|" + String(f0r) + "|" + String(f0on) + "|" + String(f1p) + "|" + String(f1r) + "|" + String(f1on);
  // Add Memories
  retMsg += " RAM:" + String(m0u) + "|" + String(m0t) + "|" + String(m1u) + "|" + String(m1t);
  return retMsg;
}

String SensorsHandler::sensor0message(uint32_t epoch) {
  String retMsg = "{\"temperature\": ";
  retMsg += String(s0t);
  retMsg += ", \"humidity\": ";
  retMsg += String(s0h);
  retMsg += ", \"_timestamp\": ";
  retMsg += String(epoch);
  retMsg += "}";
  return retMsg;
}

String SensorsHandler::sensor1message(uint32_t epoch) {
  String retMsg = "{\"temperature\": ";
  retMsg += String(s1t);
  retMsg += ", \"humidity\": ";
  retMsg += String(s1h);
  retMsg += ", \"_timestamp\": ";
  retMsg += String(epoch);
  retMsg += "}";
  return retMsg;
}

String SensorsHandler::fan0message(uint32_t epoch) {
  String retMsg = "{\"power\": ";
  retMsg += String(f0p);
  retMsg += ", \"rpm\": ";
  retMsg += String(f0r);
  retMsg += ", \"IsOn\": ";
  retMsg += String(f0on);
  retMsg += ", \"_timestamp\": ";
  retMsg += String(epoch);
  retMsg += "}";
  return retMsg;
}

String SensorsHandler::fan1message(uint32_t epoch) {
  String retMsg = "{\"power\": ";
  retMsg += String(f1p);
  retMsg += ", \"rpm\": ";
  retMsg += String(f1r);
  retMsg += ", \"IsOn\": ";
  retMsg += String(f1on);
  retMsg += ", \"_timestamp\": ";
  retMsg += String(epoch);
  retMsg += "}";
  return retMsg;
}

String SensorsHandler::memory0message(uint32_t epoch) {
  String retMsg = "{\"total\": ";
  retMsg += String(m0t);
  retMsg += ", \"free\": ";
  retMsg += String(m0u);
  retMsg += ", \"_timestamp\": ";
  retMsg += String(epoch);
  retMsg += "}";
  return retMsg;
}

String SensorsHandler::memory1message(uint32_t epoch) {
  String retMsg = "{\"total\": ";
  retMsg += String(m1t);
  retMsg += ", \"free\": ";
  retMsg += String(m1u);
  retMsg += ", \"_timestamp\": ";
  retMsg += String(epoch);
  retMsg += "}";
  return retMsg;
}

String SensorsHandler::rangeTmessage(uint32_t epoch) {
  String retMsg = "{\"min\": ";
  retMsg += String(s0t);
  retMsg += ", \"max\": ";
  retMsg += String(s0h);
  retMsg += ", \"_timestamp\": ";
  retMsg += String(epoch);
  retMsg += "}";
  return retMsg;
}
