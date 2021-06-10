#include "LcdPages.h"
#include <stdio.h>

//----------------------------------------------------------------
// Constructor and Destructor
//----------------------------------------------------------------
LcdPages::LcdPages() {
  //_lcd = &lcd;
  degreeFix();
  //updateLcd();
}
LcdPages::LcdPages(LiquidCrystal_PCF8574_Mod& lcd) {
  _lcd = &lcd;
  isAttached = true;
  degreeFix();
  createSpecialChars();
  updateLcd();
}
//LcdPages::LcdPages(LiquidCrystal& lcd) {
//  _lcd = &lcd;
//  LcdPages::degreeFix();
//  LcdPages::updateLcd();
//}
//LcdPages::LcdPages(LiquidCrystal& lcd, uint8_t cols, uint8_t rows) {
//  _lcd = &lcd;
//  this->_cols = cols;
//  this->_rows = rows;
//  _lcd->begin(cols, rows);
//  LcdPages::degreeFix();
//  LcdPages::updateLcd();
//}
//LcdPages::LcdPages(uint8_t address, uint8_t cols, uint8_t rows) {
//  LiquidCrystalI2C_RS_EN(_lcd, address, false)
//  // for i2c variants, this must be called first.
//  Wire.begin();
//  // set up the LCD's number of columns and rows, must be called.
//  this->_cols = cols;
//  this->_rows = rows;
//  _lcd->begin(cols, rows);
//  LcdPages::updateLcd();
//}

/* The write function is needed for derivation from the Print class. */
inline size_t LcdPages::write(uint8_t ch)
{
  //_send(ch, true);
  _lcd->write(ch);
  return 1; // assume sucess
} // write()


//----------------------------------------------------------------
// Private String updaters
//----------------------------------------------------------------
void LcdPages::degreeFix() {
  this->p0_0[6] = 0xDF;
  this->p0_0[7] = ' ';
  this->p0_0[14] = 0xDF;
  this->p0_0[15] = ' ';
  this->p1_1[6] = 0xDF;
  this->p1_1[7] = ' ';
  this->p1_1[14] = 0xDF;
  this->p1_1[15] = ' ';
}

void LcdPages::createSpecialChars() {
  int char0[] = {B01110, B10001, B01110, B10001, B00100, B01010, B00000, B00100}; // WiFi character
  int char1[] = {B10001, B11011, B10101, B10001, B01100, B10010, B10010, B01101}; // MQTT character
  this->_lcd->createChar(0, char0);
  this->_lcd->createChar(1, char1);
}

void LcdPages::changeActualTempString() {
	// Temperature Sensor 0
	char t0c[4]{};
  sprintf(t0c, "%4d", this->_t0);
  char t0c0 = t0c[0];
  char t0c1 = t0c[1];
  char t0c2 = t0c[2];
  char t0c3 = t0c[3];
	this->p0_0[3] = t0c1;
	this->p0_0[4] = t0c2;
	this->p0_0[5] = t0c3;

	// Temperature Sensor 1
	char t1c[4]{};
	sprintf(t1c, "%4d", this->_t1);
  char t1c0 = t1c[0];
  char t1c1 = t1c[1];
  char t1c2 = t1c[2];
  char t1c3 = t1c[3];
	this->p0_0[11] = t1c1;
	this->p0_0[12] = t1c2;
	this->p0_0[13] = t1c3;

  //Check what page is active
  if (this->_actualPage == 0) { updateLcd(); }
}

void LcdPages::changeFanString() {
	// Fan Duty Percentage
	if (_isOn) {
    char fanc[4]{};
    sprintf(fanc, "%4d", this->_fanPerc);
    char c0 = fanc[0];
    char c1 = fanc[1];
    char c2 = fanc[2];
    char c3 = fanc[3];
    this->p0_1[4] = c1;
    this->p0_1[5] = c2;
    this->p0_1[6] = c3;
    this->p0_1[7] = '%';
	} else {
    this->p0_1[4] = ' ';
    this->p0_1[5] = 'O';
		this->p0_1[6] = 'F';
		this->p0_1[7] = 'F';
	}
  // Time Stamp
  char hhc[3]{};
  char mmc[3]{};
  sprintf(hhc, "%2d", this->_hh);
  sprintf(mmc, "%2d", this->_mm);
  this->p0_1[9] = (char)hhc[0];
  this->p0_1[10] = (char)hhc[1];
  this->p0_1[11] = (char)mmc[0];
  this->p0_1[12] = (char)mmc[1];

  //Check what page is active
  if (this->_actualPage == 0) { updateLcd(); }
}

void LcdPages::changeTempRangeString() {
	// MIN Temperature
	char mint[4]{};
	sprintf(mint, "%4d", this->_tmin);
  char c0 = mint[0];
  char c1 = mint[1];
  char c2 = mint[2];
  char c3 = mint[3];
	this->p1_1[4] = c2;
	this->p1_1[5] = c3;

	// MAX Temperature
	char maxt[4]{};
	sprintf(maxt, "%4d", this->_tmax);
  char c0x = maxt[0];
  char c1x = maxt[1];
  char c2x = maxt[2];
  char c3x = maxt[3];
	this->p1_1[12] = c2x;
	this->p1_1[13] = c3x;

  //Check what page is active
  if (this->_actualPage == 1) { updateLcd(); }
}

void LcdPages::changeTempRangeString(byte tmin, byte tmax) {
	// MIN Temperature
	char mint[4]{};
	sprintf(mint, "%4d", tmin);
  char c0 = mint[0];
  char c1 = mint[1];
  char c2 = mint[2];
  char c3 = mint[3];
	this->p1_1[4] = mint[2];
	this->p1_1[5] = mint[3];

	// MAX Temperature
	char maxt[4]{};
	sprintf(maxt, "%4d", tmax);
  char c0x = maxt[0];
  char c1x = maxt[1];
  char c2x = maxt[2];
  char c3x = maxt[3];
	this->p1_1[12] = c2x;
	this->p1_1[13] = c3x;

  updateLcd();
}

//----------------------------------------------------------------
// Private LCD updaters
//----------------------------------------------------------------
bool LcdPages::updateLcd() {
  if (isAttached) {
  	this->_lcd->clear();
	  switch (this->_actualPage) {
	  case 0: // Status page
  		this->_lcd->noBlink();
	  	this->_lcd->setCursor(0, 0);
		  this->_lcd->print(this->p0_0);
  		this->_lcd->setCursor(0, 1);
  		this->_lcd->print(this->p0_1);
      this->updateIotChars();
  		break;
  	case 1: // Temperature range view
  		this->_lcd->noBlink();
  		this->_lcd->setCursor(0, 0);
  		this->_lcd->print(this->p1_0);
  		this->_lcd->setCursor(0, 1);
  		this->_lcd->print(this->p1_1);
  		break;
  	case 2: // Temperature MIN Edit
  		this->_lcd->setCursor(0, 0);
  		this->_lcd->print(this->p1_0);
  		this->_lcd->setCursor(0, 1);
  		this->_lcd->print(this->p1_1);
  		this->_lcd->setCursor(5, 1);
  		this->_lcd->blink();
  	case 3: // Temperature MAX Edit
  		this->_lcd->setCursor(0, 0);
  		this->_lcd->print(this->p1_0);
	  	this->_lcd->setCursor(0, 1);
  		this->_lcd->print(this->p1_1);
  		this->_lcd->setCursor(13, 1);
  		this->_lcd->blink();
  	}
    return true;
  } else {
    return false;
  }
}

void LcdPages::updateIotChars() {
  this->_lcd->setCursor(14, 1);
  if (_isWifi) {
    this->_lcd->write(byte(0));
  } else {
    this->_lcd->write('-');
  }
  this->_lcd->setCursor(15, 1);
  if (_isMqtt) {
    this->_lcd->write(byte(1));
  } else {
    this->_lcd->write('-');
  }
}


//----------------------------------------------------------------
// Public Value updaters
//----------------------------------------------------------------
void LcdPages::updateTemperatureRange(byte tmin, byte tmax) {
  if (this->_tmin != tmin || this->_tmax != tmax){
    this->_tmin = tmin;
    this->_tmax = tmax;
    this->changeTempRangeString();
  }
}

void LcdPages::updateSensorValues(byte t0, byte t1) {
  if (this->_t0 != t0 || this->_t1 != t1) {
	  this->_t0 = t0;
	  this->_t1 = t1;
	  this->changeActualTempString();
  }
}

void LcdPages::updateSensorValues(byte t0, byte h0, byte t1, byte h1) {
  this->_h0 = h0;
  this->_h1 = h1;
  if (this->_t0 != t0 || this->_t1 != t1) {
    this->_t0 = t0;
    this->_t1 = t1;
    this->changeActualTempString();
  }
}

void LcdPages::updateFanStatus(byte fanPerc, bool isOn) {
  if (this->_fanPerc != fanPerc || this->_isOn != isOn) {
    this->_fanPerc = fanPerc;
    this->_isOn = isOn;
    this->changeFanString();
  }
}

void LcdPages::updateTimeStamp(byte hh, byte mm) {
  if(this->_hh != hh || this->_mm != mm) {
    this->_hh = hh;
    this->_mm = mm;
    this->changeFanString();
  }
}

void LcdPages::updateIotStatus(bool isWifiConnected, bool isMqttConnected) {
  _isWifi = isWifiConnected;
  if (_isWifi) {
    _isMqtt = isMqttConnected;
  } else {
    _isMqtt = false; // MQQT can NOT be connected if the WiFi are not available
  }
  //Check what page is active
  if (this->_actualPage == 0) { updateLcd(); }
}
//----------------------------------------------------------------
// Public Value updaters
//----------------------------------------------------------------
bool LcdPages::buttonPressed(byte btn) {
	bool res = false;
	switch (this->_actualPage) {
	case 0: // Status page
		// Only up & down button work
		if (btn == BUTTON_UP || btn == BUTTON_DOWN) {
			this->_actualPage = 1;
			res = true;
		}
		else {
			res = false;
		}
		break;
	case 1: // Temperature Range page
		// Up & Down go back to status page
		if (btn == BUTTON_UP || btn == BUTTON_DOWN) {
			this->_actualPage = 0;
			res = true;
		}
		// OK to enable change min temperature
		else if (btn == BUTTON_OK) {
			this->_ttemp = this->_tmin; // set temp value
			this->_actualPage = 2;
			res = true;
		}
		// Canc button do nothing
		else {
			res = false;
		}
		break;
	case 2: // Min temperature change page
		if (btn == BUTTON_UP) {
			if (this->_ttemp < 99) this->_ttemp += 1;
			changeTempRangeString(this->_ttemp, this->_tmax);
			res = true;
		}
		if (btn == BUTTON_DOWN) {
			if (this->_ttemp > 0) this->_ttemp -= 1;
			changeTempRangeString(this->_ttemp, this->_tmax);
			res = true;
		}
		if (btn == BUTTON_OK) {
			this->_tmin = this->_ttemp; // accept value
			this->_ttemp = this->_tmax; // set new temp value
			changeTempRangeString(this->_tmin, this->_tmax);
			this->_actualPage = 3; // go to next value
			res = true;
		}
		if (btn == BUTTON_CANC) {
			changeTempRangeString(this->_tmin, this->_tmax);
			this->_actualPage = 1; // exit editing
			res = true;
		}
		break;
	case 3: // Max temperature change page
		if (btn == BUTTON_UP) {
			if (this->_ttemp < 99) this->_ttemp += 1;
			changeTempRangeString(this->_tmin, this->_ttemp);
			res = true;
		}
		if (btn == BUTTON_DOWN) {
			if (this->_ttemp > 0) this->_ttemp -= 1;
			changeTempRangeString(this->_tmin, this->_ttemp);
			res = true;
		}
		if (btn == BUTTON_OK) {
			this->_tmax = this->_ttemp; // accept value
			changeTempRangeString(this->_tmin, this->_tmax);
			this->_actualPage = 1; // exit editing
			res = true;
		}
		if (btn == BUTTON_CANC) {
			changeTempRangeString(this->_tmin, this->_tmax);
			this->_actualPage = 1; // exit editing
			res = true;
		}
		break;
	default:
		res = false;
		break;
	}

	// Update LCD
	if (res) { updateLcd(); }
	return res;
}
