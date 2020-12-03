#include <SimpleDHT.h>
#include <Adafruit_LiquidCrystal.h>
#include <EEPROM.h>
#include <SimpleRelay.h>
#include "FanControl_Pins.h"

// PWM output for the fan
#define PWM6 OCR4D      // Pin 6 shortcut
#define PWM6_MAX OCR4C  // Terminal count


// Tells the amount of time (in ms) to wait between updates
const int PWM_SAMPLETIME = 2000;
const int DHT_SAMPLETIME = 2000;
const int USB_SAMPLETIME = 1000;
const long debounceDelay = 250;

const int DUTY_MIN = 64;        // The minimum fans speed (0...255)
const int DUTY_DEAD_ZONE = 64;  // The delta between the minimum output for the PID and DUTY_MIN (DUTY_MIN - DUTY_DEAD_ZONE).

/* RPM calculation */
volatile unsigned long duration = 0; // accumulates pulse width
volatile unsigned int pulsecount = 0;
volatile unsigned long previousMicros = 0;
int ticks = 0, speed = 0;

unsigned long prevPwm, prevDht, prevUsb, lastDebounceTime = 0; // Time placeholders

double duty;
// Display temp, .5 rounded and Compute temp, integer (declared as double because of PID library input);
//double dtemp, ctemp;
//double targetTemp = 40;

// Sensors Values
byte Temperature1 = 0;
byte Temperature2 = 0;
byte Humidity1 = 0;
byte Humidity2 = 0;

// LCD Pages Strings
int actualPage = 1;
byte TargetTempMin = 25;
byte TargetTempMax = 35;
byte TempMod = 0;
String LcdDHT = "";
String LcdStatus = "";
String LcdTargetMinTitle = "Fan Min Temp.   ";
String LcdTargetMaxTitle = "Fan Max Temp.   ";
String LcdTarget = "";
String DebugDHT = "";
bool LcdNeedUpdate = true;


// Initialize all the libraries.
SimpleDHT11 sensor1(TEMP1_IN), sensor2(TEMP2_IN);
Adafruit_LiquidCrystal lcd(Lcd_RS, Lcd_Enable, Lcd_D4, Lcd_D5, Lcd_D6, Lcd_D7);
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

/* Custom LCD Characters */
const byte degCelsius[] = {B11000, B11000, B00111, B01000, B01000, B01000, B01000, B00111};
const byte usbChar[] = {B00100, B01110, B00100, B10101, B10101, B01101, B00110, B00100};

void setup()
{
  // Setup LCD and display a message
  lcd.begin(16, 2);
  lcd.createChar(0, degCelsius);
  lcd.createChar(1, usbChar);
  lcd.setCursor(0, 0);
  lcd.print("Starting up ...");

  //Set the chip for USB connection check
  USBCON|=(1<<OTGPADE); //enables VBUS pad
  checkUsb();

  //pinMode(SPD_IN, INPUT);
  //pinMode(RELAY, OUTPUT);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_CANC, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);

  // RPM reading interrupt
  //attachInterrupt(digitalPinToInterrupt(SPD_IN), pickRPM, FALLING);

  pwm6configure();

  lcd.setCursor(0, 1);
  lcd.print("Fans............");
  DEBUG("Fans...");
  fanRelay.on();
  pwmSet6(255);
  // Let the fan run for 5s. Here we could add a fan health control to see if the fan revs to a certain value.
  delay(5000);
  fanRelay.off();

  lcd.setCursor(0, 1);
  lcd.print("Sensors.........");
  DEBUG("Sensor...");
  GetSensorData(&sensor1, &Temperature1, &Humidity1);
  GetSensorData(&sensor2, &Temperature2, &Humidity2);
  FormatDhtString();
  DEBUG(DebugDHT);
  DEBUG("\n");
  delay(1000);
  
  // Check the avalaibility of stored values
  lcd.setCursor(0, 1);
  lcd.print("Stored Values...");
  DEBUG("Stored Values...");
  GetConfigFromEeprom(&TargetTempMin, &TargetTempMax);
  if (!isTargetValid()) {
    TargetTempMin = 25;
    TargetTempMax = 35;
    SetConfigToEeprom(&TargetTempMin, &TargetTempMax);
  }
  DEBUG("TargetMin: ");
  DEBUG(TargetTempMin);
  DEBUG("TargetMax: ");
  DEBUG(TargetTempMax);
  DEBUG("\n");
  delay(1000);
  
  lcd.setCursor(0, 1);
  lcd.print("Ready to go!    ");
  DEBUG("Ready.\n\n");
  delay(1000);

  prevPwm = millis();
  prevDht = prevPwm;
  prevUsb = prevPwm;
}

void loop()
{
  unsigned long cur = millis();

  bool up = !digitalRead(BTN_UP);
  bool down = !digitalRead(BTN_DOWN);
  bool canc = !digitalRead(BTN_CANC);
  bool ok = !digitalRead(BTN_OK);

  if (cur - prevUsb >= USB_SAMPLETIME) {
      checkUsb();
  }
  
  if (cur - prevDht >= DHT_SAMPLETIME)
  {
    byte t = 0, h = 0;
    byte data[40] = {0};
    prevDht = cur;

    GetSensorData(&sensor1, &Temperature1, &Humidity1);
    GetSensorData(&sensor2, &Temperature2, &Humidity2);
    FormatDhtString();
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

    FormatStatusString();
    
    DEBUG("TargetTempMin: ");
    DEBUG(TargetTempMin);
    DEBUG(" TargetTempMax: ");
    DEBUG(TargetTempMax);
    DEBUG(" Duty: ");
    DEBUG(duty);
    DEBUG("\n");
  }


  //filter out any noise by setting a time buffer
  if ( (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis(); //set the current time
    if (up) {buttonClick(1); DEBUG("Button Up"); DEBUG("\n");}
    if (down) {buttonClick(2); DEBUG("Button Down"); DEBUG("\n");}
    if (canc) {buttonClick(4); DEBUG("Button Canc"); DEBUG("\n");}
    if (ok) {buttonClick(3); DEBUG("Button OK"); DEBUG("\n");}

    up = HIGH;
    down = HIGH;
    canc = HIGH;
    ok = HIGH;
  }

  // Update the LCD
  if (LcdNeedUpdate) { updateLCD(); }
}
