//---------------------------------------------------------------------------------------------------
/* Arduino libraries */
//---------------------------------------------------------------------------------------------------
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
//#include <SSLClient.h>
#include <Arduino.h>
#include <Wire.h>
#include <CircularBuffer.h>
#include <TimeLib.h>

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
//const int BTNS = A1;
const int BT_UP = 2, BT_DW = 3, BT_OK = 9, BT_ES = 10;
volatile byte lastBtnPressed = 0;
void btnUpPressed() { lastBtnPressed = 11; } // UP   botton
void btnDwPressed() { lastBtnPressed = 22; } // DOWN botton
void btnOkPressed() { lastBtnPressed = 33; } // OK   botton
void btnEsPressed() { lastBtnPressed = 44; } // ESC  botton


// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
int status = WL_IDLE_STATUS;
WiFiClient    wifiClient;            // Used for the TCP socket connection
PubSubClient psClient(wifiClient);
WiFiUDP Udp;    // A UDP instance to let us send and receive packets over UDP

// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
const unsigned long seventyYears = 2208988800UL;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

long lastReconnectAttempt = 0;
long lastTopicPublish = 0;
long lastTimeCheck = 0;

// Tells the amount of time (in ms) to wait between updates
const long BUTTON_SAMPLETIME = 200;
const long UART_SAMPLETIME = 500;
const long TOPICPUSH_TIME = 6000;
const long NTP_SAMPLETIME = 600000;

unsigned long prevI2C = 0, prevButton = 0, prevNTP = 0; // Time placeholders
const byte uartMsgSize = 35;
byte uartMsgAvailable = 0;
CircularBuffer<char,250> uartBuff;           // Serial buffer
SensorsHandler sens;                         // Sensor helper class

// Sensors Values
byte lastT0 = 0, lastT1 = 0, lastTmin = 0, lastTmax = 0;

// LCD Pages Strings
int actualPage = 1;
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
  
  // Pin setup
  //pinMode(BTNS, INPUT);
  pinMode(BT_UP, INPUT_PULLUP);
  pinMode(BT_DW, INPUT_PULLUP);
  pinMode(BT_OK, INPUT_PULLUP);
  pinMode(BT_ES, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BT_UP), btnUpPressed, LOW);
  attachInterrupt(digitalPinToInterrupt(BT_DW), btnDwPressed, LOW);
  attachInterrupt(digitalPinToInterrupt(BT_OK), btnOkPressed, LOW);
  attachInterrupt(digitalPinToInterrupt(BT_ES), btnEsPressed, LOW);
  Debug::println("-> Buttons configured");

  // Start the serial on the board
  Serial1.begin(57600);
  Debug::println("-> Serial1 started");

  // MQTT Setup
  psClient.setServer(mqttSERVER, mqttPORT);
  psClient.setCallback(callback);
  Debug::println("-> MQTT configured");

  // WiFi NTP Setup
  Debug::println("-> Connecting to WiFi...");
  connectWiFi(networkSSID, networkPASSWORD);
  Udp.begin(udpLocalPort);
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServer); 
  setSyncProvider(getNtpTime);
  //Debug::println("-> NTP configured");

  // Allow the hardware to sort itself out
  delay(1500);
  lastReconnectAttempt = 0;
  lastTopicPublish = 0;
  Debug::println("Ready!");
}

void loop()
{
  long nowSensors = millis();
  uint32_t nowTime = now();

  // Check the buttons
  if (nowSensors - prevButton > BUTTON_SAMPLETIME) {
    prevButton = nowSensors;
    byte btn = lastBtnPressed;
    lastBtnPressed = 0;  // Reset the status
    if (btn != 0) {
      lpg.buttonPressed(btn);
      Debug::print("Last Pressed Button: ");
      Debug::println(btn);
    }
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
          if (sens.fromUartMessage(tmp, nowTime)) { uartMsgAvailable = 1; }
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
    lpg.updateTimeStamp((byte)hour(), (byte)minute());
    Debug::print("TimeStamp: ");
    Debug::print((byte)hour());
    Debug::print(" | ");
    Debug::print((byte)minute());
    lpg.updateIotStatus(WiFi.status() == WL_CONNECTED, psClient.connected());
    Debug::print(" | WiFi connected: ");
    Debug::print(WiFi.status() == WL_CONNECTED);
    Debug::print("  MQTT connected: ");
    Debug::println(psClient.connected());
  }

  // Check WiFi Connection
  if (WiFi.status() != WL_CONNECTED) {
    if (connectWiFi(networkSSID, networkPASSWORD)) {
      Udp.begin(udpLocalPort);
      //get a random server from the pool
      WiFi.hostByName(ntpServerName, timeServer); 
      setSyncProvider(getNtpTime);
    }
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
      // Send the plc message to the mqtt server
      if (uartMsgAvailable == 1) {
        psClient.publish(mqttTopicPlc, sens.plcmessage().c_str());
        uartMsgAvailable = 0;
      }
      // Publish the sensors topics when needed or at max interval
      if (lastT0 != sens.sensor0temperature() || lastT1 != sens.sensor1temperature() || nowSensors - lastTopicPublish > TOPICPUSH_TIME) {
        // Temperatures changed from the last time
        psClient.publish(mqttTopicSensor0, sens.sensor0message(nowTime).c_str());
        psClient.publish(mqttTopicSensor1, sens.sensor1message(nowTime).c_str());
        psClient.publish(mqttTopicFan0, sens.fan0message(nowTime).c_str());
        psClient.publish(mqttTopicFan1, sens.fan1message(nowTime).c_str());
        psClient.publish(mqttTopicMemory0, sens.memory0message(nowTime).c_str());
        psClient.publish(mqttTopicMemory1, sens.memory1message(nowTime).c_str());
        lastT0 = sens.sensor0temperature();
        lastT1 = sens.sensor1temperature();
        lastTopicPublish = nowSensors;
        Debug::print("MQTT Publised with clock: ");
        Debug::print((timeStatus() != timeNotSet) ? "SYNC  " : "ERROR ");
        Debug::print("DateTime: ");
        Debug::print(digitalClockDisplay());
        Debug::print("  Unix format: ");
        Debug::println(nowTime);
      }
      // Publish the range when needed
      if (lastTmin != sens.temperatureRangeMin() || lastTmax != sens.temperatureRangeMax() || nowSensors - lastTopicPublish > TOPICPUSH_TIME * 10) {
        // Temperature range changed
        psClient.publish(mqttTopicRangeT, sens.rangeTmessage(nowTime).c_str());
        lastTmin = sens.temperatureRangeMin();
        lastTmax = sens.temperatureRangeMax();
      }
      psClient.loop();
    } // MQTT Client connected
    // Check Time sync packets
    if(nowSensors - prevNTP > NTP_SAMPLETIME) {
      prevNTP = nowSensors;
      Debug::print((timeStatus() != timeNotSet) ? "SYNC  " : "ERROR ");
      Debug::print("DateTime: ");
      Debug::print(digitalClockDisplay());
      Debug::print("  Unix format: ");
      Debug::println(nowTime);
    }
  } // WiFi Connected

}

//---------------------------------------------------------------------------------------------------
/* Internal functions */
//---------------------------------------------------------------------------------------------------

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress &address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Debug::println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Debug::println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
     return secsSince1900 - seventyYears;
    }
  }
  Debug::println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

String digitalClockDisplay(){
  // digital clock display of the time
  String resTime = "";
  resTime += printDigits(year());
  resTime += "-";
  resTime += printDigits(month());
  resTime += "-";
  resTime += printDigits(day());
  resTime += " ";
  resTime += printDigits(hour());
  resTime += ":";
  resTime += printDigits(minute());
  resTime += ":";
  resTime += printDigits(second());
  return resTime;
}

String printDigits(int digits){
  // utility for digital clock display
  String res = "";
  if(digits < 10){
    res = "0" + (String)digits;
  } else {
    res = (String)digits;
  }
  return res;
}
