//---------------------------------------------------------------------------------------------------
/* Arduino libraries */
//---------------------------------------------------------------------------------------------------
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
//#include <SSLClient.h>
#include <Arduino.h>
#include <Wire.h>
#include <CircularBuffer.h>

//---------------------------------------------------------------------------------------------------
// Declare structs
//---------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------
// Declare functions that are inside an external header file
//---------------------------------------------------------------------------------------------------
void callback(char* topic, byte* payload, unsigned int length);
bool reconnect(WiFiClient* wifi, PubSubClient* psClient, char* clientID, char* user, char* pass, char* inTopic);
bool connectWiFi(char* ssid, char* pass);


//---------------------------------------------------------------------------------------------------
/* Local headers */
//---------------------------------------------------------------------------------------------------
#include "Debug.h"
#include "priv/credentials.h"
//#include "priv/certificates.h"  // Needed for the SSL connection
#include "LiquidCrystal_PCF8574_Mod.h"
#include "LcdPages.h"
#include "MemoryFree.h"
#include "mqttUtil.h"               // This header contain the functions related to the MQTT
#include "SensorsHandler.h"

//---------------------------------------------------------------------------------------------------
/* Pin declarations */
//---------------------------------------------------------------------------------------------------
const int BTNS = A1;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
int status = WL_IDLE_STATUS;
WiFiClient    wifiClient;            // Used for the TCP socket connection
PubSubClient psClient(wifiClient);

long lastReconnectAttempt = 0;
long lastTopicPublish = 0;

// Tells the amount of time (in ms) to wait between updates
const long BUTTON_SAMPLETIME = 200;
const long UART_SAMPLETIME = 500;
const long TOPICPUSH_TIME = 5000;

unsigned long prevI2C = 0, prevButton = 0; // Time placeholders
const byte uartMsgSize = 35;
CircularBuffer<char,250> uartBuff;           // Serial buffer
SensorsHandler sens;                         // Sensor helper class

// Sensors Values
byte lastT0 = 0, lastT1 = 0, lastTmin = 0, lastTmax = 0;

// LCD Pages Strings
int actualPage = 1;
//byte TargetTempMin = 25;
//byte TargetTempMax = 35;
byte TempMod = 0;


// Initialize the LCD library
//LiquidCrystalI2C_RS_EN(lcd, 0x27, false)
LiquidCrystal_PCF8574_Mod lcd(0x27);
LcdPages lpg;


void setup()
{
  delay(2000);
  Debug::println("Starting.......");

  // Setup LCD and display a message
  Wire.begin();
  lcd.begin(16, 2);
  lcd.setBacklight(127);
  lcd.print("hello over i2c!");
  Debug::println("-> Base LCD object initialized.");
  lpg = LcdPages(lcd);
  delay(1000);
  Debug::println("-> LCD configured");
  
  // MQTT Setup
  psClient.setServer(mqttSERVER, mqttPORT);
  psClient.setCallback(callback);
  Debug::println("-> MQTT configured");
  
  // Pin setup
  pinMode(BTNS, INPUT);
  Debug::println("-> Buttons configured");

  // Start the serial on the board
  Serial1.begin(57600);
  Debug::println("-> Serial1 started");
  
  // Allow the hardware to sort itself out
  delay(1500);
  lastReconnectAttempt = 0;
  lastTopicPublish = 0;
  Debug::println("Ready!");
}

void loop()
{
  long nowSensors = millis();

  // Check the buttons
  if (nowSensors - prevButton > BUTTON_SAMPLETIME) {
    prevButton = nowSensors;
    int btnVal = analogRead(BTNS);
    byte btnPress = getPressedButton(btnVal);
    lpg.buttonPressed(btnPress);
    Debug::print("Analog read: ");
    Debug::print(btnVal);
    Debug::print("  Button: ");
    Debug::println(btnPress);
  }

  // Check uart connection
  if (nowSensors - prevI2C > UART_SAMPLETIME) {
    prevI2C = nowSensors;
    // Get the available chars from serial and push then in the buffer
    while(Serial1.available()) {
      char c = Serial1.read();
      uartBuff.push(c);
      Debug::print(c);
    }
    // Scan the buffer for the starting char
    while(!uartBuff.isEmpty()) {
      if (uartBuff.first() == '$') { // Buffer start with the start id char
        if(uartBuff.size() > uartMsgSize) {   // Buffer is big enough to store the entire string
          // Store the chars to send to sensor handler parser
          String tmp;
          char tc;
          for (byte i = 0; i <= uartMsgSize; i++) {
            tc = (char)uartBuff.shift();
            tmp += tc;
            if (tc == ';') { break; }
          }
          // Send the string to the sensor handler
          sens.fromUartMessage(tmp);
          break; // exit while message readed
        } else {
          break; // need to recieve more chars from the serial
        }
      } else {
        uartBuff.shift(); // discard the first char
      }
    }
    sens.setFreeMemory0(freeMemory());
    Debug::println(sens.debugString());
    // Update Lcd values;
    lastT0 = sens.sensor0temperature();
    lastT1 = sens.sensor1temperature();
    lpg.updateSensorValues(sens.sensor0temperature(), sens.sensor1temperature());
    lpg.updateTemperatureRange(sens.temperatureRangeMin(), sens.temperatureRangeMax()); 
    lpg.updateFanStatus(sens.fan0power(), sens.fan0isOn());
    lpg.updateIotStatus(WiFi.status() == WL_CONNECTED, psClient.connected());
    Debug::print("WiFi connected: ");
    Debug::print(WiFi.status() == WL_CONNECTED);
    Debug::print("  MQTT connected: ");
    Debug::println(psClient.connected());
  }

  // Check WiFi Connection
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi(networkSSID, networkPASSWORD);
  } else {  // WiFi Connected
    // Check MQTT Connection
    if (!psClient.connected()) {
      long nowWIFI = millis();
      if (nowWIFI - lastReconnectAttempt > 500) {
        lastReconnectAttempt = nowWIFI;
        //Attempt to reconnect
        if (reconnect(wifiClient, psClient, mqttCLIENTID, mqttUSERNAME, mqttPASSWORD, mqttTopicRoot)) {
          lastReconnectAttempt = 0;
        }
      }
    
    } else { // MQTT Client connected
      // Publish the sensors topics when needed or at max interval
      if (lastT0 != sens.sensor0temperature() || lastT1 != sens.sensor1temperature() || nowSensors - lastTopicPublish > TOPICPUSH_TIME) {
        // Temperatures changed from the last time
        psClient.publish(mqttTopicSensor0, sens.sensor0message().c_str());
        psClient.publish(mqttTopicSensor1, sens.sensor1message().c_str());
        psClient.publish(mqttTopicFan0, sens.fan0message().c_str());
        psClient.publish(mqttTopicFan1, sens.fan1message().c_str());
        psClient.publish(mqttTopicMemory0, sens.memory0message().c_str());
        psClient.publish(mqttTopicMemory1, sens.memory1message().c_str());
        lastT0 = sens.sensor0temperature();
        lastT1 = sens.sensor1temperature();
        lastTopicPublish = nowSensors;
      }
      // Publish the range when needed
      if (lastTmin != sens.temperatureRangeMin() || lastTmax != sens.temperatureRangeMax()) {
        // Temperature range changed
        psClient.publish(mqttTopicRangeT, sens.rangeTmessage().c_str());
        lastTmin = sens.temperatureRangeMin();
        lastTmax = sens.temperatureRangeMax();
      }
      psClient.loop();
    } // MQTT Client connected
  } // WiFi Connected

}

byte getPressedButton(int value) {
  if (value >= 180 && value <= 200) { return 11; }  // UP   botton
  if (value >= 355 && value <= 375) { return 44; }  // CANC button
  if (value >= 540 && value <= 560) { return 33; }  // OK   button
  if (value >= 750 && value <= 770) { return 22; }  // DOWN button
  return 0;
}
