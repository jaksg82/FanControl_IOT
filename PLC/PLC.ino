/* Arduino libraries */
#include <SimpleDHT.h>
#include <EEPROM.h>
#include <SimpleRelay.h>
//#include <Wire.h>

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
    String outstr = "$STAT,";
    outstr = outstr + Temperature1 + "," + Humidity1 + ",";
    outstr = outstr + Temperature2 + "," + Humidity2 + ",";
    outstr = outstr + TargetTempMin + "," + TargetTempMax + ",";
    outstr = outstr + fanPerc + "," + (fanRelay.isRelayOn() ? "Y" : "N") + ",";
    outstr = outstr + rpm0 + "," + ticks0 + ",";
    outstr = outstr + freeMemory() + ";";
    Serial1.println(outstr);
    Debug::println(outstr);
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
// i2c comm functions
//---------------------------------------------------------------------------------------
//void sendAnswer(bool isRange = false) {
//void sendAnswer() {
//    if (isRange) { // Send Temperature range message
//        responseMsg[0] = 'R';
//        responseMsg[1] = 'N';
//        responseMsg[2] = 'G';
//        responseMsg[3] = TargetTempMin;
//        responseMsg[4] = TargetTempMax;
//        responseMsg[5] = '$';
//        responseMsg[6] = '$';
//        responseMsg[7] = '$';
//        responseMsg[8] = '$';
//        responseMsg[9] = '$';
//        responseMsg[10] = '$';
//    }
//    else {         // Send Status message
//        byte fanPerc = (duty / 255) * 100;
//		    int ram = freeMemory();
//        responseMsg[0] = 'S';
//        responseMsg[1] = 'T';
//        responseMsg[2] = 'S';
//        responseMsg[3] = Temperature1;
//        responseMsg[4] = Humidity1;
//        responseMsg[5] = Temperature2;
//        responseMsg[6] = Humidity2;
//        responseMsg[7] = fanPerc;
//        responseMsg[8] = fanRelay.isRelayOn();
//        responseMsg[9] = (ram >> 8) & 0xFF;
//        responseMsg[10] = ram & 0xFF;
//    }
//    Wire.write(responseMsg, ResponseSize);
//    for (int i = 0; i < ResponseSize; i++) {
//      Debug::print(responseMsg[i]);
//    }
//    Debug::println();
//}
//
//void readRequest() {
//    Debug::print("Message: ");
//    for (int i = 0; i < RequestSize; i++) {
//        requestMsg[i] = Wire.read();
//        Debug::print(requestMsg[i]);
//    }
//    Debug::println();
//    // Parse the message for the specific request
//    if (requestMsg[0] == 'S') {          // Set messages
//        if (requestMsg[3] == 'T') {      // Set temperature range
//            byte tmin = EepromUtil::FitInTemp(requestMsg[4]);
//            byte tmax = EepromUtil::FitInTemp(requestMsg[5]);
//            if (EepromUtil::isTargetValid(tmin, tmax)) {
//                if (TargetTempMin != tmin || TargetTempMax != tmax) {
//                    TargetTempMin = tmin;
//                    TargetTempMax = tmax;
//                    EepromUtil::SetConfigToEeprom(&TargetTempMin, &TargetTempMax);
//                }
//            }
//            //sendAnswer(true);            // Send back the range value accepted
//        }
//    }
//    else if (requestMsg[0] == 'G') {     // Get messages
//        if (requestMsg[3] == 'S') {      // Get status message
//            sendAnswer();                // Reply with the status message
//        }
//        else if (requestMsg[3] == 'R') {
//            //sendAnswer(true);            // Reply with the range message
//        }
//    }
//}


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
