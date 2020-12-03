#ifndef LCDPAGES_H
#define LCDPAGES_H

#include <Arduino.h>
#include <LiquidCrystalIO.h>

// When using the I2C version, these two extra includes are always needed. Doing this reduces the memory slightly for
// users that are not using I2C.
#include <IoAbstractionWire.h>
#include <Wire.h>

#define BUTTON_UP 11
#define BUTTON_DOWN 22
#define BUTTON_OK 33
#define BUTTON_CANC 44


class LcdPages {
  private:
    byte _totalPages = 2;
	byte _actualPage = 0;
	byte _t0 = 0;
    byte _h0 = 0;
    byte _t1 = 0;
    byte _h1 = 0;
    byte _tmin = 0;
    byte _tmax = 0;
	byte _fanPerc = 0;
	bool _isOff = true;
	//  16 chars   0000000000111111  
	//             0123456789012345
    string p0_0 = "T0:   ° T1:   ° ";
	string p0_1 = "Fan:    %   ON  ";
	string p1_0 = "Temperatures    ";
    string p1_1 = "Min:  ° Max:  ° ";
	
	// pointer to the lcd display class
	LiquidCrystal* _lcd;
	
	// String updaters
	void changeActualTempString();
	void changeFanString();
	void changeTempRangeString();
	
	
  public:
    // Constructor
    LcdPages(LiquidCrystal& lcd);
		
	// Inputs
	void updateTemperatureRange(byte tmin, byte tmax);
	void updateSensorValues(byte t0, byte t1);
	void updateSensorValues(byte t0, byte h0, byte t1, byte h1);
	void updateFanStatus(byte fanPerc, bool isOff);
	
	// Page handling
	bool buttonPressed(byte btn);
	
	
};

#endif