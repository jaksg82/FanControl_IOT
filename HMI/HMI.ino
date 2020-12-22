//---------------------------------------------------------------------------------------------------
/* Arduino libraries */
//---------------------------------------------------------------------------------------------------
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
//#include <SSLClient.h>
#include <Arduino.h>
//#include <LiquidCrystalIO.h>
//#include <IoAbstractionWire.h>
#include <Wire.h>

//---------------------------------------------------------------------------------------------------
// Declare structs
//---------------------------------------------------------------------------------------------------
// struct MqttParams {
//   char clientID[64];
//   char userName[64];
//   char userPass[64];
//   char inTopic[64];
// };

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
#include "pinConfig.h"
#include "priv/credentials.h"
//#include "priv/certificates.h"  // Needed for the SSL connection
#include "LiquidCrystal_PCF8574_Mod.h"
#include "LcdPages.h"
#include "MemoryFree.h"
#include "mqttUtil.h"               // This header contain the functions related to the MQTT


// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
int status = WL_IDLE_STATUS;
WiFiClient    wifiClient;            // Used for the TCP socket connection
PubSubClient psClient(wifiClient);

long lastReconnectAttempt = 0;
long lastTopicPublish = 0;

// Tells the amount of time (in ms) to wait between updates
const long debounceDelay = 250;
const long I2C_SAMPLETIME = 2000;

unsigned long prevI2C, lastDebounceTime = 0; // Time placeholders

// Sensors Values
byte sens0t = 0, sens0h = 0;
byte sens1t = 0, sens1h = 0;
byte fan0perc = 0;
bool fan0isOn = false;
int mem0used = 0, mem0total = 2048; // RAM Memory of ProMicro board
int mem1used = 0, mem1total = 4000; // RAM Memory of Nano 33 Iot board

// LCD Pages Strings
int actualPage = 1;
byte TargetTempMin = 25;
byte TargetTempMax = 35;
byte TempMod = 0;
bool LcdNeedUpdate = true;

// Initialize the LCD library
//LiquidCrystalI2C_RS_EN(lcd, 0x27, false)
LiquidCrystal_PCF8574_Mod lcd(0x27);
LcdPages lpg;// = LcdPages(lcd);
//LcdPages lpg(lcd);

void setup()
{
  delay(2000);
  Debug::println("Starting.......");
  
  // Setup LCD and display a message
  //lcd.configureBacklightPin(3);
  //lcd.backlight();
  Wire.begin();
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.print("hello over i2c!");
  //lcd.noBacklight();
  Debug::println("-> Base LCD object initialized.");
  //LcdPages lpg(lcd);
  //LcdPages lpg = LcdPages(lcd);
  lpg = LcdPages(lcd);
  delay(1000);
  //lcd.backlight();
  Debug::println("-> LCD configured");
  
  // MQTT Setup
  psClient.setServer(mqttSERVER, mqttPORT);
  psClient.setCallback(callback);
  Debug::println("-> MQTT configured");
  
  // Pin setup
  // pinMode(BTN_UP, INPUT_PULLUP);
  // pinMode(BTN_DOWN, INPUT_PULLUP);
  // pinMode(BTN_CANC, INPUT_PULLUP);
  // pinMode(BTN_OK, INPUT_PULLUP);
  Debug::println("-> Buttons configured");

  //prevPwm = millis();
  //prevDht = prevPwm;
  //prevUsb = prevPwm;

  //Wire.begin();
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

  // Check uart connection
  if (nowSensors - prevI2C > 2000) {
    while(Serial1.available()) {
      char c = Serial1.read();
      Debug::print(c);
    }
    //Debug::println();
    prevI2C = nowSensors;
    //lcd.updateLcd();
    byte rnd = random(10, 35);
    Debug::print("Random value: ");
    Debug::println(rnd);
    lpg.updateSensorValues(rnd, rnd);
  }

  // Check WiFi Connection
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi(networkSSID, networkPASSWORD);
  }

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
    
  } else {
    // Client connected
    if (nowSensors - lastTopicPublish > 2000) {
      lastTopicPublish = nowSensors;
//      String s0string = getSensor0payload();
//      char s0chars[s0string.length()+2];
//      s0string.toCharArray(s0chars, s0string.length()+1);
      psClient.publish("qiot/things/simone/NanoIOT/sensor0", "{actual: 25}");
//      Debug::println("Sensor 0 Publish");
      Debug::println("qiot/things/simone/NanoIOT/sensor0");
//      Debug::println(s0chars);

    }
    psClient.loop();
  }


}
