/* Local hardware libraries */
#include <Wire.h>
//#include <Adafruit_LiquidCrystal.h>
#include "pinCfg.h"

/* SSL, TLS, MQTT libraries and configs */
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <SSLClient.h>
#include "credentials.h"
#include "certificates.h"

SSLClientParameters mTLS = SSLClientParameters::fromPEM(pemClientCert, sizeof pemClientCert, pemPrivateKey, sizeof pemPrivateKey);

int status = WL_IDLE_STATUS;
IPAddress server(74,125,232,128);
IPAddress clientIP;
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

/*  Callback function to handle the incoming messages from the MQTT broker */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiSSLClient wfClient;
SSLClient ethClientSSL(wfClient, TAs, (size_t)TAs_NUM, A5, 1, SSLClient::SSL_INFO);
PubSubClient psClient(mqttSERVER, mqttPORT, callback, ethClientSSL);


// Tells the amount of time (in ms) to wait between updates
//const int USB_SAMPLETIME = 1000;
const long debounceDelay = 250;

unsigned long prevUsb, lastDebounceTime = 0; // Time placeholders

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
  //lcd.begin(16, 2);
  //lcd.createChar(0, degCelsius);
  //lcd.createChar(1, usbChar);
  //lcd.setCursor(0, 0);
  //lcd.print("Starting up ...");

  // Enable mutual TLS with SSLClient
  ethClientSSL.setMutualAuthParams(mTLS);

// connect to ardiuino.cc over ssl (port 443 for websites)
ethClientSSL.connect("www.arduino.cc", 443);
// Make a HTTP request
ethClientSSL.println("GET /asciilogo.txt HTTP/1.1");
ethClientSSL.println("User-Agent: AdafruitFeatherM0WiFi");
ethClientSSL.print("Host: ");
ethClientSSL.println(server);
ethClientSSL.println("Connection: close");
ethClientSSL.println();
ethClientSSL.flush();


  //Set the chip for USB connection check
  //USBCON|=(1<<OTGPADE); //enables VBUS pad
  //checkUsb();

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_CANC, INPUT_PULLUP);
  pinMode(BTN_OK, INPUT_PULLUP);

  // Check the fans
  //lcd.setCursor(0, 1);
  //lcd.print("Fans............");
  //DEBUG("Fans...");
  //fanRelay.on();
  //pwmSet6(255);
  // Let the fan run for 5s. Here we could add a fan health control to see if the fan revs to a certain value.
  //delay(5000);
  //fanRelay.off();

  //lcd.setCursor(0, 1);
  //lcd.print("Sensors.........");
  //DEBUG("Sensor...");
  //GetSensorData(&sensor1, &Temperature1, &Humidity1);
  //GetSensorData(&sensor2, &Temperature2, &Humidity2);
  //FormatDhtString();
  //DEBUG(DebugDHT);
  //DEBUG("\n");
  //delay(1000);
  
  // Check the avalaibility of stored values
  //lcd.setCursor(0, 1);
  //lcd.print("Stored Values...");
  //DEBUG("Stored Values...");
  //GetConfigFromEeprom(&TargetTempMin, &TargetTempMax);
  //if (!isTargetValid()) {
  //  TargetTempMin = 25;
  //  TargetTempMax = 35;
  //  SetConfigToEeprom(&TargetTempMin, &TargetTempMax);
  //}
  //DEBUG("TargetMin: ");
  //DEBUG(TargetTempMin);
  //DEBUG("TargetMax: ");
  //DEBUG(TargetTempMax);
  //DEBUG("\n");
  //delay(1000);
  
  //lcd.setCursor(0, 1);
  //lcd.print("Ready to go!    ");
  //DEBUG("Ready.\n\n");
  //delay(1000);

  //prevPwm = millis();
  //prevDht = prevPwm;
  //prevUsb = prevPwm;
}

void loop()
{
  unsigned long cur = millis();

  bool up = !digitalRead(BTN_UP);
  bool down = !digitalRead(BTN_DOWN);
  bool canc = !digitalRead(BTN_CANC);
  bool ok = !digitalRead(BTN_OK);

  if (!psClient.connected()) {
    reconnect();
  }
  psClient.loop();

/*
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
*/
  // Update the LCD
  //if (LcdNeedUpdate) { updateLCD(); }
}
