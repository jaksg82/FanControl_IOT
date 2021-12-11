#ifndef LCDPAGES_H
#define LCDPAGES_H

#include "Arduino.h"
#include "Print.h"
#include <stddef.h>
#include <stdint.h>
#include "LiquidCrystal_PCF8574_Mod.h"
#include "ByteRange.h"

// When using the I2C version, these two extra includes are always needed. Doing this reduces the memory slightly for
// users that are not using I2C.
//#include <LiquidCrystalIO.h>
//#include <IoAbstractionWire.h>
//#include <Wire.h>

#define BUTTON_UP 11
#define BUTTON_DOWN 22
#define BUTTON_OK 33
#define BUTTON_CANC 44

class LcdPages : public Print {
  private:
  //byte _totalPages = 4;
  byte _actualPage = 0;
  byte _t0 = 0;
  byte _h0 = 0;
  byte _t1 = 0;
  byte _h1 = 0;
  byte _tmin = 0;
  byte _tmax = 0;
  byte _fanPerc = 0;
  bool _isOn = false;
  uint16_t _yy = 0;
  byte _mt = 0;
  byte _dd = 0;
  byte _hh = 0;
  byte _mm = 0;
  byte _ss = 0;
  ByteRange modTemp = ByteRange(0, 99);
  
  #ifdef LCD_SCREEN_1602A
  bool _isLCD1602 = true;
  //  16 chars     00000000001111111111
  //               01234567890123456789
  char p0_0[18] = "T0:     T1:     ";
  char p0_1[18] = "Fan:   % 0000   ";
  char p1_0[18] = "Temperatures    ";
  char p1_1[18] = "Min:    Max:    ";
  #else
  bool _isLCD1602 = false;
  //  20 chars     00000000001111111111
  //               01234567890123456789
  char p0_0[22] = "2021-12-08 21:16:55 ";
  char p0_1[22] = "T0: 00 C   T1: 00 C ";
  char p0_2[22] = "Fan: 000%           ";
  char p0_3[22] = "Range: 00 <====> 00 ";
  #endif
    
  bool _isEditPage = false;
  bool _isWifi = false;
  bool _isMqtt = false;

  // lcd display class
  //LiquidCrystal* _lcd;
  LiquidCrystal_PCF8574_Mod* _lcd;
  uint8_t _address, _cols, _rows;
  bool isAttached = false;

  // String updaters
  void degreeFix();
  void createSpecialChars();
  void changeActualTempString();
  void changeFanString();
  void changeTempRangeString();
  void changeTempRangeString(byte tmin, byte tmax, byte curPos);
  void changeTimeString();

  // LCD content updater
  bool updateLcd();
  void updateIotChars();

  public:
    // Constructor
    //LcdPages(LiquidCrystal& lcd);
    LcdPages();
    LcdPages(LiquidCrystal_PCF8574_Mod& lcd);
    //LcdPages(LiquidCrystal& lcd, uint8_t cols, uint8_t rows);
    //LcdPages(uint8_t address, uint8_t cols, uint8_t rows);

  // Inputs
  void updateTemperatureRange(byte tmin, byte tmax);
  void updateSensorValues(byte t0, byte t1);
  void updateSensorValues(byte t0, byte h0, byte t1, byte h1);
  void updateFanStatus(byte fanPerc, bool isOn);
  void updateIotStatus(bool isWifiConnected, bool isMqttConnected);
  void updateTimeStamp(byte hh, byte mm);
  void updateTimeStamp(uint16_t yy, byte mt, byte dd, byte hh, byte mm, byte ss);
  
  // Page handling
  ByteRange buttonPressed(byte btn);

  // support of Print class
  virtual size_t write(uint8_t ch);
  using Print::write;

};

#endif
