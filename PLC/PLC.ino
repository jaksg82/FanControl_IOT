#include <SimpleDHT.h>
#include <EEPROM.h>
#include <SimpleRelay.h>
#include <Wire.h>
#include "pinConfig.h"
#include "lib/Debug.h"
#include "lib/eepromUtil.h"

// PWM output for the fan
#define PWM6 OCR4D      // Pin 6 shortcut
#define PWM6_MAX OCR4C  // Terminal count

/* PWM calculations */
const int DUTY_MIN = 64;        // The minimum fans speed (0...255)
const int DUTY_DEAD_ZONE = 64;  // The delta between the minimum output for the PID and DUTY_MIN (DUTY_MIN - DUTY_DEAD_ZONE).
double duty;                    // The actual pwm value

/* RPM calculation */
volatile unsigned long duration = 0; // accumulates pulse width
volatile unsigned int pulsecount = 0;
volatile unsigned long previousMicros = 0;
int ticks = 0, speed = 0;

/* I2C Comm Configs */
const int NodeReadDelay = 100;
const byte NodeAddress = 1;
const byte ResponseSize = 10;
const byte RequestSize = 6;
byte responseMsg[ResponseSize];
byte requestMsg[RequestSize];

/* Tells the amount of time (in ms) to wait between updates */
const int PWM_SAMPLETIME = 2000;
const int DHT_SAMPLETIME = 2000;
const int USB_SAMPLETIME = 1000;
const long debounceDelay = 250;
unsigned long prevPwm, prevDht, prevUsb, prevComm, lastDebounceTime = 0; // Time placeholders


// Sensors Values
byte Temperature1 = 0;
byte Temperature2 = 0;
byte Humidity1 = 0;
byte Humidity2 = 0;
byte TargetTempMin = 25;
byte TargetTempMax = 35;


// Initialize all the libraries.
SimpleDHT11 sensor1(TEMP1_IN), sensor2(TEMP2_IN);
SimpleRelay fanRelay = SimpleRelay(RELAY);

/* Configure the PWM clock */
void pwm6configure()
{
  // TCCR4B configuration
  TCCR4B = 4; /* 4 sets 23437Hz */

  // TCCR4C configuration
  TCCR4C = 0;

  // TCCR4D configuration
  TCCR4D = 0;

  // PLL Configuration
  PLLFRQ = (PLLFRQ & 0xCF) | 0x30;

  // Terminal count for Timer 4 PWM
  OCR4C = 255;
}

// Set PWM to D6 (Timer4 D)
// Argument is PWM between 0 and 255
void pwmSet6(int value)
{
  OCR4D = value;  // Set PWM value
  DDRD |= 1 << 7; // Set Output Mode D7
  TCCR4C |= 0x09; // Activate channel D
}

/* Called when hall sensor pulses */
void pickRPM ()
{
  volatile unsigned long currentMicros = micros();

  if (currentMicros - previousMicros > 20000) // Prevent pulses less than 20k micros far.
  {
    duration += currentMicros - previousMicros;
    previousMicros = currentMicros;
    ticks++;
  }
}


void setup()
{
  // Set the needed pins
 
  // RPM reading interrupt
  //attachInterrupt(digitalPinToInterrupt(SPD_IN), pickRPM, FALLING);

  // Configure the clock on Timer 4
  pwm6configure();

  Debug::println("Fans...");
  fanRelay.on();
  pwmSet6(255);
  // Let the fan run for 5s. Here we could add a fan health control to see if the fan revs to a certain value.
  delay(5000);
  fanRelay.off();

  // Get the first read of sensor
  Debug::println("Sensor...");
  GetSensorData(&sensor1, &Temperature1, &Humidity1);
  GetSensorData(&sensor2, &Temperature2, &Humidity2);
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
  
  Debug::println("Ready.\n\n");
  delay(1000);

  prevPwm = millis();
  prevDht = prevPwm;
  prevUsb = prevPwm;
  prevComm = prevPwm;
  // Activate i2c
  Wire.begin(NodeAddress);
}

void loop()
{
  unsigned long cur = millis();

  //bool up = !digitalRead(BTN_UP);
  //bool down = !digitalRead(BTN_DOWN);
  //bool canc = !digitalRead(BTN_CANC);
  //bool ok = !digitalRead(BTN_OK);

  //if (cur - prevUsb >= USB_SAMPLETIME) {
  //    checkUsb();
  //}
  
  if (cur - prevDht >= DHT_SAMPLETIME)
  {
    byte t = 0, h = 0;
    byte data[40] = {0};
    prevDht = cur;

    GetSensorData(&sensor1, &Temperature1, &Humidity1);
    GetSensorData(&sensor2, &Temperature2, &Humidity2);
    //FormatDhtString();
  }
  
  if (cur - prevPwm >= PWM_SAMPLETIME)
  {
    prevPwm = cur;

    // Compute the duty cicle
    byte maxTemp = max(Temperature1, Temperature2);
    duty = ((maxTemp - TargetTempMin) * ((255 - DUTY_MIN) / (TargetTempMax - TargetTempMin))) + DUTY_MIN;
    // fit the duty in the range 0-255
    if (duty < 0) { duty = 0; }
    if (duty > 255) { duty = 255; }
    
    // Turn the fans ON/OFF
    if (round(duty) < DUTY_MIN)
    {
      fanRelay.off();
      PWM6 = 0;
      //fanRunning = false;
    }
    else
    {
      //fanRunning = true;
      PWM6 = duty;
      fanRelay.on();
    }

    //FormatStatusString();
    
    Debug::print("TargetTempMin: ");
    Debug::print(TargetTempMin);
    Debug::print(" TargetTempMax: ");
    Debug::print(TargetTempMax);
    Debug::print(" Duty: ");
    Debug::println(duty);
  }


  ////filter out any noise by setting a time buffer
  //if ( (millis() - lastDebounceTime) > debounceDelay) {
  //  lastDebounceTime = millis(); //set the current time
  //  if (up) {buttonClick(1); DEBUG("Button Up"); DEBUG("\n");}
  //  if (down) {buttonClick(2); DEBUG("Button Down"); DEBUG("\n");}
  //  if (canc) {buttonClick(4); DEBUG("Button Canc"); DEBUG("\n");}
  //  if (ok) {buttonClick(3); DEBUG("Button OK"); DEBUG("\n");}

  //  up = HIGH;
  //  down = HIGH;
  //  canc = HIGH;
  //  ok = HIGH;
  //}

  // Check the i2c
  if (cur - prevComm >= NodeReadDelay) {
      if (Wire.available() >= RequestSize) {
          // Read the request message
      }
  }
}

//---------------------------------------------------------------------------------------
// i2c comm functions
//---------------------------------------------------------------------------------------
void readRequest() {
    for (int i = 0; i < RequestSize; i++) {
        requestMsg[i] = Wire.read();
    }
    // Parse the message for the specific request
    if (requestMsg[0] == 'S') {          // Set messages
        if (requestMsg[3] == 'T') {      // Set temperature range
            byte tmin = EepromUtil::FitInTemp(requestMsg[4]);
            byte tmax = EepromUtil::FitInTemp(requestMsg[5]);
            if (EepromUtil::isTargetValid(tmin, tmax)) {
                if (TargetTempMin != tmin || TargetTempMax != tmax) {
                    TargetTempMin = tmin;
                    TargetTempMax = tmax;
                    EepromUtil::SetConfigToEeprom(&TargetTempMin, &TargetTempMax);
                }
            }
            sendAnswer(true);            // Send back the range value accepted
        }
    }
    else if (requestMsg[0] == 'G') {     // Get messages
        if (requestMsg[3] == 'S') {      // Get status message
            sendAnswer();                // Reply with the status message
        }
        else if (requestMsg[3] == 'R') {
            sendAnswer(true);            // Reply with the range message
        }
    }
}

void sendAnswer(bool isRange = false) {
    if (isRange) { // Send Temperature range message
        responseMsg[0] = 'R';
        responseMsg[1] = 'N';
        responseMsg[2] = 'G';
        responseMsg[3] = TargetTempMin;
        responseMsg[4] = TargetTempMax;
        responseMsg[5] = '$';
        responseMsg[6] = '$';
        responseMsg[7] = '$';
        responseMsg[8] = '$';
        responseMsg[9] = '$';
    }
    else {         // Send Status message
        byte fanPerc = (duty / 255) * 100;
        responseMsg[0] = 'S';
        responseMsg[1] = 'T';
        responseMsg[2] = 'S';
        responseMsg[3] = Temperature1;
        responseMsg[4] = Humidity1;
        responseMsg[5] = Temperature2;
        responseMsg[6] = Humidity2;
        responseMsg[7] = fanPerc;
        responseMsg[8] = fanRelay.isRelayOn();
        responseMsg[9] = '$';

    }
    Wire.write(responseMsg, ResponseSize);
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
