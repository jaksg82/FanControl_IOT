/* Arduino libraries */
#include <SimpleDHT.h>
#include <EEPROM.h>
#include <SimpleRelay.h>
//#include <Wire.h>
//#include <stdio.h>

/* Local headers */
#include "timer4settings.h"
#include "Debug.h"
#include "eepromUtil.h"
#include "MemoryFree.h"

/*        Pinouts       */
const int RPM_IN = 3;        // RPM input pin on interrupt0
const int RELAY = 9;         // Relay output pin
const int TEMP1_IN = 20;     // Temperature sensor input pin
const int TEMP2_IN = 21;     // Temperature sensor input pin

/* PWM calculations */
double duty;                    // The actual pwm value

/* RPM calculation */
volatile unsigned long pulsecount0 = 0;
unsigned long ticks0 = 0;
int rpm0 = 0;
void pickRpm0() {
  pulsecount0++;
}

/* Uart Comm Configs */
const byte RequestSize = 6;
byte requestMsg[RequestSize] {};

/* Tells the amount of time (in ms) to wait between updates */
const int PWM_SAMPLETIME = 2000;
const int DHT_SAMPLETIME = 2000;
const int UART_SAMPLETIME = 2000;
const long debounceDelay = 250;
unsigned long prevPwm, prevDht, prevUart, prevComm, lastDebounceTime = 0; // Time placeholders


// Sensors Values
byte Temperature1 = 0;
byte Temperature2 = 0;
byte Humidity1 = 0;
byte Humidity2 = 0;
byte TargetTempMin = 25;
byte TargetTempMax = 35;
byte t0series[5] {0, 0, 0, 0, 0};
byte t1series[5] {0, 0, 0, 0, 0};

// Initialize all the libraries.
SimpleDHT11 sensor1(TEMP1_IN), sensor2(TEMP2_IN);
SimpleRelay fanRelay = SimpleRelay(RELAY);

//-------------------------------------------------------------------------------
// Startup configurations
//-------------------------------------------------------------------------------
void setup()
{
  // Configure the clock on Timer 4
  pwm6configure();

  Debug::println("Fans...");
  fanRelay.on();
  pwmSet6(255);
  // Let the fan run for 5s. Here we could add a fan health control to see if the fan revs to a certain value.
  attachInterrupt(digitalPinToInterrupt(RPM_IN), pickRpm0, FALLING);
  for (int i = 0; i < 5; i++) {
    delay(1000); // Let the fans run for 1 second
    UpdateRpmValues(1000);
  }
  fanRelay.off();

  // Get the first read of sensor
  Debug::println("Sensor...");
  GetSensorData(&sensor1, &Temperature1, &Humidity1);
  GetSensorData(&sensor2, &Temperature2, &Humidity2);
  t0series[0] = Temperature1;
  t0series[1] = Temperature1;
  t0series[2] = Temperature1;
  t0series[3] = Temperature1;
  t0series[4] = Temperature1;
  t1series[0] = Temperature2;
  t1series[1] = Temperature2;
  t1series[2] = Temperature2;
  t1series[3] = Temperature2;
  t1series[4] = Temperature2;
  Debug::print("T1: ");
  Debug::print(Temperature1);
  Debug::print(" H1: ");
  Debug::print(Humidity1);
  Debug::print(" T2: ");
  Debug::print(Temperature2);
  Debug::print(" H2: ");
  Debug::println(Humidity2);
  delay(1000);
  
  // Check the avalaibility of stored values
  Debug::println("Stored Values...");
  EepromUtil::GetConfigFromEeprom(&TargetTempMin, &TargetTempMax);
  if (!EepromUtil::isTargetValid(TargetTempMin, TargetTempMax)) {
    TargetTempMin = 25;
    TargetTempMax = 35;
    EepromUtil::SetConfigToEeprom(&TargetTempMin, &TargetTempMax);
  }
  Debug::print("TargetMin: ");
  Debug::print(TargetTempMin);
  Debug::print("TargetMax: ");
  Debug::println(TargetTempMax);
  delay(1000);

  // Open the serial com onboard
  Serial1.begin(57600);
  delay(1000);

  Debug::println("Ready.\n\n");

  prevPwm = millis();
  prevDht = prevPwm;
  prevUart = prevPwm;
  prevComm = prevPwm;
}

//-------------------------------------------------------------------------------
// Main program to repeat
//-------------------------------------------------------------------------------
void loop()
{
  unsigned long cur = millis();

  //------------------------------------------------------------
  // Update the sensor values
  //------------------------------------------------------------
  if (cur - prevDht >= DHT_SAMPLETIME)
  {
    byte t = 0, h = 0;
    byte data[40] = {0};
    prevDht = cur;
    GetSensorData(&sensor1, &Temperature1, &Humidity1);
    GetSensorData(&sensor2, &Temperature2, &Humidity2);
    // update the series
    for (int i = 0; i < 4; i++) {
      t0series[i] = t0series[i+1];
      t1series[i] = t1series[i+1];
    }
    t0series[4] = Temperature1;
    t1series[4] = Temperature2;
  }
  
  //------------------------------------------------------------
  // Update the PWM value
  //------------------------------------------------------------
  if (cur - prevPwm >= PWM_SAMPLETIME)
  {
    prevPwm = cur;
    // Compute the duty cicle
    float avg0 = (t0series[0] + t0series[1] + t0series[2] + t0series[3] + t0series[4]) / 5;
    float avg1 = (t1series[0] + t1series[1] + t1series[2] + t1series[3] + t1series[4]) / 5;
    float maxTemp = max(avg0, avg1);
    byte DutyMin = 64, DutyMax = 255;  // DutyMin approx. 25%
    duty = ((maxTemp - TargetTempMin) * ((DutyMax - DutyMin) / (TargetTempMax - TargetTempMin))) + DutyMin;
    // fit the duty in the range 0-255
    if (duty < 0) { duty = 0; }
    if (duty > 255) { duty = 255; }
    
    // Turn the fans ON/OFF
    if (round(duty) < DutyMin)
    {
      pwmSet6(0);
      fanRelay.off();
    }
    else
    {
      pwmSet6(duty);
      fanRelay.on();
    }
    Debug::print("TargetTempMin: ");
    Debug::print(TargetTempMin);
    Debug::print(" TargetTempMax: ");
    Debug::print(TargetTempMax);
    Debug::print(" Duty: ");
    Debug::println(duty);

    // Get the RPMs
    UpdateRpmValues(PWM_SAMPLETIME);
  }
  
  //------------------------------------------------------------
  // Send the status through the serial
  //------------------------------------------------------------
  if (cur - prevUart >= UART_SAMPLETIME)
  {
    prevUart = cur;
    byte fanPerc = (duty / 255) * 100;
    Serial1.print("$STAT,");
    Debug::print("$STAT,");
    ByteToHex(Temperature1);
    ByteToHex(Humidity1);
    ByteToHex(Temperature2);
    ByteToHex(Humidity2);
    ByteToHex(TargetTempMin);
    ByteToHex(TargetTempMax);
    ByteToHex(fanPerc);
    Serial1.print(fanRelay.isRelayOn() ? "Y" : "N");
    Debug::print(fanRelay.isRelayOn() ? "Y" : "N");
    Int16ToHex(rpm0);
    IntToHex(freeMemory());
    Serial1.print(";");
    Debug::print(";");
    //Debug::println(sizeof(rpm0));
    
  }

  //------------------------------------------------------------
  // Check for incoming requests on the serial
  //------------------------------------------------------------
  //if (cur - prevComm >= NodeReadDelay) {
      if (Serial1.available() >= RequestSize) {
          // Read the request message
          Debug::println("Request Available");
          //readRequest();
      }
  //}

}

//---------------------------------------------------------------------------------------
// Format string functions
//---------------------------------------------------------------------------------------
void ByteToHex(byte value) {
  char valChars[2]{};
  sprintf(valChars, "%.2X", value);
  Serial1.print(valChars);
  Debug::print(valChars);
}

void Int16ToHex(int16_t value) {
  char valChars[4]{};
  sprintf(valChars, "%.4X", value);
  Serial1.print(valChars);
  Debug::print(valChars);
}
void IntToHex(int value) {
  char valChars[4]{};
  sprintf(valChars, "%.4X", value);
  Serial1.print(valChars);
  Debug::print(valChars);
}

void UInt16ToHex(uint16_t value) {
  char valChars[4]{};
  sprintf(valChars, "%.4X", value);
  Serial1.print(valChars);
  Debug::print(valChars);
}

void Int32ToHex(int32_t value) {
  char valChars[8]{};
  sprintf(valChars, "%.8X", value);
  Serial1.print(valChars);
  Debug::print(valChars);
}

//---------------------------------------------------------------------------------------
// Temperature sensor functions
//---------------------------------------------------------------------------------------
int GetSensorData(SimpleDHT11* pSensor, byte* ptemperature, byte* phumidity) {
    byte t = 0, h = 0;
    byte pdata[40] = { 0 };
    if (!pSensor->read(&t, &h, pdata))
    {
        Debug::print("Temperature: ");
        Debug::println(t);

        if (!isnan(t)) { *ptemperature = t; }
        if (!isnan(h)) { *phumidity = h; }
        return 1;
    }
    else
    {
        // If there's an error in the sensor, wait 5 seconds to let the communication reset
        Debug::println("Temperature: Error");
        return 0;
    }

}

//---------------------------------------------------------------------------------------
// RPM interrupt
//---------------------------------------------------------------------------------------
void UpdateRpmValues(int sampleMillis) {
  detachInterrupt(digitalPinToInterrupt(RPM_IN)); // Detach to avoid conflicts
  ticks0 = pulsecount0; // Store the counter
  pulsecount0 = 0; // Restart the counter
  attachInterrupt(digitalPinToInterrupt(RPM_IN), pickRpm0, FALLING); // Enable the interrupt
  if (ticks0 > 0) {
    rpm0 = (ticks0 / 2) * 60 / (sampleMillis / 1000); // Convert from Hz to RPM
    Debug::print("RPM: ");
    Debug::println(rpm0);
  } else {
    rpm0 = 0;
    Debug::println("Fans not working!!!");
  }
}
