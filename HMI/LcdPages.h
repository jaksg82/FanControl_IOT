#ifndef LCDPAGES_H
#define LCDPAGES_H

#include "Arduino.h"
#include "Print.h"
#include <stddef.h>
#include <stdint.h>
#include "LiquidCrystal_PCF8574_Mod.h"

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
  byte _ttemp = 0;
  byte _fanPerc = 0;
  bool _isOn = false;
  byte _hh = 0;
  byte _mm = 0;
  //  16 chars     0000000000111111  
  //               0123456789012345
  char p0_0[20] = "T0:     T1:     ";
	char p0_1[20] = "Fan:   % 0000   ";
	char p1_0[20] = "Temperatures    ";
  char p1_1[20] = "Min:    Max:    ";
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
	void changeTempRangeString(byte tmin, byte tmax);

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
 
	// Page handling
	bool buttonPressed(byte btn);
	
  // support of Print class
  virtual size_t write(uint8_t ch);
  using Print::write;
	
};

#endif
