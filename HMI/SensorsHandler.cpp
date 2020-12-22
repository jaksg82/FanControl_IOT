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


bool SensorsHandler::fromUartMessage(char* message, size_t msgSize){
  if (msgSize >= 30) {}
  
}
