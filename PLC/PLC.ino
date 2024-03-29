
//---------------------------------------------------------------------------------------------------
// Arduino libraries
//---------------------------------------------------------------------------------------------------
#include <SimpleDHT.h>
#include <EEPROM.h>
#include <SimpleRelay.h>
#include <CircularBuffer.h>
#include <MemoryUsage.h>

//---------------------------------------------------------------------------------------------------
// Declare functions
//---------------------------------------------------------------------------------------------------
float t0avg();
float t1avg();

//---------------------------------------------------------------------------------------------------
// Local headers
//---------------------------------------------------------------------------------------------------
#include "timer4settings.h"
#include "Debug.h"
#include "eepromUtil.h"

//---------------------------------------------------------------------------------------------------
// Constant declarations
//---------------------------------------------------------------------------------------------------
const uint8_t yesValue = 111;
const uint8_t noValue = 222;

/*        Pinouts       */
const int RPM0 = 2;        // RPM input pin on interrupt0
const int RPM1 = 3;        // RPM input pin on interrupt1
const int RELAY0 = 15;         // Relay output pin
const int RELAY1 = 14;         // Relay output pin
const int TEMP1_IN = 20;     // Temperature sensor input pin
const int TEMP2_IN = 21;     // Temperature sensor input pin

/* PWM calculations */
double duty;                    // The actual pwm value

/* RPM calculation by tick counter */
volatile unsigned long pulsecount0 = 0, pulsecount1 = 0;
unsigned long ticks0 = 0, ticks1 = 0;
int rpm0 = 0, rpm1 = 0;
void pickRpm0() {
  pulsecount0++;
}
void pickRpm1() {
  pulsecount1++;
}

/* RPM calculation by pwm read */
/*volatile int pwm0value = 0, prevTime0 = 0, pwm1value = 0, prevTime1 = 0;
int rpm0 = 0, rpm1 = 0;
void rising0() {
  attachInterrupt(digitalPinToInterrupt(RPM0), falling0, FALLING);
  prevTime0 = micros();
}
void falling0() {
  attachInterrupt(digitalPinToInterrupt(RPM0), rising0, RISING);
  pwm0value = micros() - prevTime0;
}
void rising1() {
  attachInterrupt(digitalPinToInterrupt(RPM1), falling1, FALLING);
  prevTime1 = micros();
}
void falling1() {
  attachInterrupt(digitalPinToInterrupt(RPM1), rising1, RISING);
  pwm1value = micros() - prevTime1;
}*/

/* Uart Comm Configs */
// Example of input string:  $TTFFFF;
const byte RequestSize = 8;
byte requestMsg[RequestSize] {};
CircularBuffer<char,50> msgBuff;

/* Tells the amount of time (in ms) to wait between updates */
const int PWM_SAMPLETIME = 2000;
const int DHT_SAMPLETIME = 2000;
const int UART_SAMPLETIME = 2000;
const long debounceDelay = 250;
unsigned long prevPwm, prevDht, prevUart, prevComm, lastDebounceTime = 0; // Time placeholders
unsigned long lastRpmRead = 0, actualRpmRead = 0;

// Sensors Values
byte Temperature1 = 0;
byte Temperature2 = 0;
byte Humidity1 = 0;
byte Humidity2 = 0;
byte TargetTempMin = 25;
byte TargetTempMax = 35;
CircularBuffer<byte,10> t0buff;
CircularBuffer<byte,10> t1buff;

// Initialize all the libraries.
SimpleDHT11 sensor1(TEMP1_IN), sensor2(TEMP2_IN);
SimpleRelay fanRelay0 = SimpleRelay(RELAY0, true);
SimpleRelay fanRelay1 = SimpleRelay(RELAY1, true);

//-------------------------------------------------------------------------------
// Startup configurations
//-------------------------------------------------------------------------------
void setup()
{
  // Configure internal pullup on the rpm pins
  pinMode(RPM0, INPUT_PULLUP);
  pinMode(RPM1, INPUT_PULLUP);
  // Configure the clock on Timer 4
  pwm6configure();

  delay(1000);

  Debug::println("Fans...");
  fanRelay0.on();
  fanRelay1.on();
  pwmSet6(255);
  // Let the fan run for 5s. Here we could add a fan health control to see if the fan revs to a certain value.
  attachInterrupt(digitalPinToInterrupt(RPM0), pickRpm0, FALLING);
  attachInterrupt(digitalPinToInterrupt(RPM1), pickRpm1, FALLING);
  //attachInterrupt(digitalPinToInterrupt(RPM0), rising0, RISING);
  //attachInterrupt(digitalPinToInterrupt(RPM1), rising1, RISING);
  actualRpmRead = millis();
  for (int i = 0; i < 5; i++) {
    delay(1000); // Let the fans run for 1 second
    UpdateRpmValues();
  }
  fanRelay0.off();
  fanRelay1.off();

  // Get the first read of sensor
  Debug::println("Sensor...");
  GetSensorData(&sensor1, &Temperature1, &Humidity1);
  GetSensorData(&sensor2, &Temperature2, &Humidity2);
  t0buff.push(Temperature1);
  t1buff.push(Temperature2);
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

  Debug::print("size of int: ");
  Debug::println(sizeof(rpm0));

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
    prevDht = cur;
    GetSensorData(&sensor1, &Temperature1, &Humidity1);
    GetSensorData(&sensor2, &Temperature2, &Humidity2);
    t0buff.push(Temperature1);
    t1buff.push(Temperature2);
    MEMORY_PRINT_FREERAM
  }
  
  //------------------------------------------------------------
  // Update the PWM value
  //------------------------------------------------------------
  if (cur - prevPwm >= PWM_SAMPLETIME)
  {
    prevPwm = cur;
    // Compute the duty cicle
    float maxTemp = max(t0avg(), t1avg());
    if(maxTemp < TargetTempMin) {
      pwmSet6(0);
      fanRelay0.off();
      fanRelay1.off();
    } else {
      float DutyMin = 64.0, DutyMax = 255.0;  // DutyMin approx. 25%
      float calc06 = mapFloat(maxTemp, TargetTempMin, TargetTempMax, 64.0, 255.0);
      duty = calc06;
      // fit the duty in the range 0-255
      if (duty < 0) { duty = 0; }
      if (duty > DutyMax) { duty = DutyMax; }
     
      // Turn the fans ON/OFF
      if (round(duty) < DutyMin) {
        pwmSet6(0);
        fanRelay0.off();
        fanRelay1.off();
      } else {
        pwmSet6(duty);
        fanRelay0.on();
        fanRelay1.on();
      }
    }
    Debug::print("TargetTempMin: ");
    Debug::print(TargetTempMin);
    Debug::print(" TargetTempMax: ");
    Debug::print(TargetTempMax);
    Debug::print(" Duty: ");
    Debug::println(duty);
    MEMORY_PRINT_FREERAM

    // Get the RPMs
    UpdateRpmValues();
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
    Serial1.print(fanRelay0.isRelayOn() ? "Y" : "N");
    Debug::print(fanRelay0.isRelayOn() ? "Y" : "N");
    Int16ToHex(rpm0);
    Serial1.print(fanRelay1.isRelayOn() ? "Y" : "N");
    Debug::print(fanRelay1.isRelayOn() ? "Y" : "N");
    Int16ToHex(rpm1);
    IntToHex(mu_freeRam());
    Serial1.print(";");
    Debug::println(";");
    MEMORY_PRINT_FREERAM
    
  }

  //------------------------------------------------------------
  // Check for incoming requests on the serial
  //------------------------------------------------------------
  if (Serial.available()) {
    // Get the available chars
    while (Serial.available()) {
      msgBuff.push((char)Serial.read());
    }
    // Scan the buffer for the starting char
    while (!msgBuff.isEmpty()) {
      if (msgBuff.first() == '$') {
        if (msgBuff.size() > RequestSize) {
          String tmp;
          char tc;
          for (byte i = 0; i <= RequestSize; i++) {
            tc = (char)msgBuff.shift();
            tmp += tc;
            if (tc == ';') { break; }
          }
          // Parse the string
          SetTargetsFromString(tmp);
          Debug::println(tmp);
          break; // exit while message readed
        } else {
          break; // need to recieve more chars from the serial
        }
      } else {
        msgBuff.shift(); // discard the first char
      }
    }
  }

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
  char valChars[8]{};
  sprintf(valChars, "%.4X", value);
  Serial1.print(valChars);
  Debug::print(valChars);
}

/*void Int32ToHex(int32_t value) {
  char valChars[8]{};
  sprintf(valChars, "%.8X", value);
  Serial1.print(valChars);
  Debug::print(valChars);
}*/

byte ParseHex4(char value) {
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

byte ParseHex8(String value) {
  byte retVal = UINT8_MAX;
  if (value.length() >= 2) {
     byte c0 = ParseHex4((char)value.charAt(0));
     byte c1 = ParseHex4((char)value.charAt(1));
     if (c0 != 255 && c1 != 255) {
      retVal = (c0 * 16) + c1;
     }
  }
  return retVal;
}

//---------------------------------------------------------------------------------------
// Temperature sensor functions
//---------------------------------------------------------------------------------------
int GetSensorData(SimpleDHT11* pSensor, byte* ptemperature, byte* phumidity) {
  byte t = 0, h = 0;
  byte pdata[40] = { 0 };
  if (!pSensor->read(&t, &h, pdata)) {
    Debug::print("Temperature: ");
    Debug::println(t);
    if (!isnan(t)) { *ptemperature = t; }
    if (!isnan(h)) { *phumidity = h; }
    return 1;
  } else {
    // If there's an error in the sensor
    Debug::println("Temperature: Error");
    return 0;
  }
}


void SetTargetsFromString(String inputStr) {
  // Example of input string:  $TTFFFF;
  if(inputStr.substring(0, 2) == "$TT") {
    byte tmpT0 = ParseHex8(inputStr.substring(3, 4));
    byte tmpT1 = ParseHex8(inputStr.substring(5, 6));
    if(tmpT0 >= 0 && tmpT0 < 99) {
      if(tmpT1 > 0 && tmpT1 <= 99) {
        if(tmpT0 < tmpT1) {
          TargetTempMin = tmpT0;
          TargetTempMax = tmpT1;
        }
      }
    }
  }
}


//---------------------------------------------------------------------------------------
// RPM interrupt
//---------------------------------------------------------------------------------------
void UpdateRpmValues() {
  detachInterrupt(digitalPinToInterrupt(RPM0)); // Detach to avoid conflicts
  detachInterrupt(digitalPinToInterrupt(RPM1)); // Detach to avoid conflicts
  lastRpmRead = actualRpmRead;
  actualRpmRead = millis();
  ticks0 = pulsecount0; // Store the counter
  pulsecount0 = 0; // Restart the counter
  ticks1 = pulsecount1; // Store the counter
  pulsecount1 = 0; // Restart the counter
  attachInterrupt(digitalPinToInterrupt(RPM0), pickRpm0, FALLING); // Enable the interrupt
  attachInterrupt(digitalPinToInterrupt(RPM1), pickRpm1, FALLING); // Enable the interrupt
  //Debug::print("Ticks0: ");
  //Debug::print(ticks0);
  //Debug::print(" Ticks1: ");
  //Debug::println(ticks1);
  //Debug::print("lastRpmRead: ");
  //Debug::print(lastRpmRead);
  //Debug::print(" actualRpmRead: ");
  //Debug::print(actualRpmRead);
  //Debug::print(" sample time: ");
  //Debug::println(actualRpmRead-lastRpmRead);
  if (ticks0 > 0) {
    rpm0 = (ticks0 / 2) * 60 / ((actualRpmRead - lastRpmRead) / 1000); // Convert from Hz to RPM
    //Debug::print("RPM0: ");
    //Debug::println(rpm0);
  } else {
    rpm0 = 0;
    //Debug::println("Fans0 not working!!!");
  }
  if (ticks1 > 0) {
    rpm1 = (ticks1 / 2) * 60 / ((actualRpmRead - lastRpmRead) / 1000); // Convert from Hz to RPM
    //Debug::print("RPM1: ");
    //Debug::println(rpm1);
  } else {
    rpm1 = 0;
    //Debug::println("Fans1 not working!!!");
  }
}

/*void UpdateRpmValues() {
  // Detach interrupts to avoid conflicts
  detachInterrupt(digitalPinToInterrupt(RPM0));
  detachInterrupt(digitalPinToInterrupt(RPM1));
  // Store the pwm values
  int lastPwm0 = pwm0value;
  int lastPwm1 = pwm1value;
  // Reattach the interrupts
  attachInterrupt(digitalPinToInterrupt(RPM0), rising0, RISING);
  attachInterrupt(digitalPinToInterrupt(RPM1), rising1, RISING);
  
  Debug::print("lastPwm0: ");
  Debug::print(lastPwm0);
  Debug::print(" lastPwm1: ");
  Debug::println(lastPwm1);

  // Compute the RPMs
  int pwmMaxTime = 42; // 1000000 micros divided by 23438Hz 
  if (lastPwm0 < pwmMaxTime){
    rpm0 = (lastPwm0 / pwmMaxTime) * 100;
  } else {
    rpm0 = 0;
  }
  if (lastPwm1 < pwmMaxTime){
    rpm1 = (lastPwm1 / pwmMaxTime) * 100;
  } else {
    rpm1 = 0;
  }
  Debug::print("RPM0: ");
  Debug::print(rpm0);
  Debug::print(" RPM1: ");
  Debug::println(rpm1);

}*/

//---------------------------------------------------------------------------------------
// Average temperatures
//---------------------------------------------------------------------------------------
float t0avg() {
  if (t0buff.isEmpty()) {
    t0buff.push(Temperature1);
    return Temperature1;
  } else {
    float avg0 = t0buff[0];
    for (byte v = 1; v < t0buff.size(); v++) {
      avg0 = (avg0 + t0buff[v]) / 2;
    }
    return avg0;
  }
}
float t1avg() {
  if (t1buff.isEmpty()) {
    t1buff.push(Temperature2);
    return Temperature2;
  } else {
    float avg1 = t1buff[0];
    for (byte v = 1; v < t1buff.size(); v++) {
      avg1 = (avg1 + t1buff[v]) / 2;
    }
    return avg1;
  }
}

    //duty = ((maxTemp - TargetTempMin) * ((DutyMax - DutyMin) / (TargetTempMax - TargetTempMin))) + DutyMin;
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
