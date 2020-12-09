/* Arduino libraries */
#include <Wire.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>

/* Local headers */
#include "Debug.h"
#include "pinConfig.h"
#include "priv/credentials.h"
#include "priv/certificates.h"


// Update these with values suitable for your network.

int status = WL_IDLE_STATUS;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient    wifiClient;            // Used for the TCP socket connection
PubSubClient psClient(wifiClient);
#include "mqttUtil.h"

long lastReconnectAttempt = 0;
long lastTopicPublish = 0;

// Tells the amount of time (in ms) to wait between updates
const long debounceDelay = 250;
const long I2C_SAMPLETIME = 2000;

unsigned long prevI2C, lastDebounceTime = 0; // Time placeholders

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
//Adafruit_LiquidCrystal lcd(Lcd_RS, Lcd_Enable, Lcd_D4, Lcd_D5, Lcd_D6, Lcd_D7);

/* Custom LCD Characters */
const byte degCelsius[] = {B11000, B11000, B00111, B01000, B01000, B01000, B01000, B00111};
const byte usbChar[] = {B00100, B01110, B00100, B10101, B10101, B01101, B00110, B00100};

void setup()
{
  // Setup LCD and display a message

  // MQTT Setup
  psClient.setServer(mqttSERVER, mqttPORT);
  psClient.setCallback(callback);

  // Pin setup
  // pinMode(BTN_UP, INPUT_PULLUP);
  // pinMode(BTN_DOWN, INPUT_PULLUP);
  // pinMode(BTN_CANC, INPUT_PULLUP);
  // pinMode(BTN_OK, INPUT_PULLUP);


  //prevPwm = millis();
  //prevDht = prevPwm;
  //prevUsb = prevPwm;

  // Allow the hardware to sort itself out
  delay(1500);
  lastReconnectAttempt = 0;
  lastTopicPublish = 0;

}

void loop()
{
  long nowSensors = millis();
  
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!psClient.connected()) {
    long nowWIFI = millis();
    if (nowWIFI - lastReconnectAttempt > 500) {
      lastReconnectAttempt = nowWIFI;
      //Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
    
  } else {
    // Client connected
    if (nowSensors - lastTopicPublish > 2000) {
      lastTopicPublish = nowSensors;
      String s0string = getSensor0payload();
      char s0chars[s0string.length()+2];
      s0string.toCharArray(s0chars, s0string.length()+1);
      psClient.publish("qiot/things/simone/NanoIOT/sensor0", s0chars);
      Debug::println("Sensor 0 Publish");
      Debug::print("qiot/things/simone/NanoIOT/sensor0");
      Debug::println(s0chars);

    }
    psClient.loop();
  }

}
