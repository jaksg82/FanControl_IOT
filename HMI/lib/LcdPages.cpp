#include "LcdPages.h"
//----------------------------------------------------------------
// Constructor and Destructor
//----------------------------------------------------------------
LcdPages::LcdPages(LiquidCrystal& lcd) {
  this->_lcd = &lcd;
}


//----------------------------------------------------------------
// Private String updaters
//----------------------------------------------------------------
void LcdPages::changeActualTempString() {
	// Temperature Sensor 0
	char t0c[4]{};
    sprintf(t0c, "%3d", this->_t0);
	this->p0_0.at(3) = t0c[1];
	this->p0_0.at(4) = t0c[2];
	this->p0_0.at(5) = t0c[3];

	// Temperature Sensor 1
	char t1c[4]{};
	sprintf(t1c, "%3d", this->_t1);
	this->p0_0.at(11) = t1c[1];
	this->p0_0.at(12) = t1c[2];
	this->p0_0.at(13) = t1c[3];
}

void LcdPages::changeFanString() {
	// Fan Duty Percentage
	char fanc[4]{};
    sprintf(fanc, "%3d", this->_fanPerc);
	this->p0_1.at(5) = fanc[1];
	this->p0_1.at(6) = fanc[2];
	this->p0_1.at(7) = fanc[3];

	if (_isOff) {
		this->p0_1.at(11) = 'O';
		this->p0_1.at(12) = 'F';
		this->p0_1.at(13) = 'F';
	} else {
		this->p0_1.at(11) = 'O';
		this->p0_1.at(12) = 'N';
		this->p0_1.at(13) = ' ';
	}
}
void LcdPages::changeTempRangeString() {
	// MIN Temperature
	char mint[4]{};
	sprintf(mint, "%3d", this->_tmin);
	this->p1_1.at(4) = mint[2];
	this->p1_1.at(5) = mint[3];

	// MAX Temperature
	char maxt[4]{};
	sprintf(maxt, "%3d", this->_tmax);
	this->p1_1.at(12) = maxt[2];
	this->p1_1.at(13) = maxt[3];
}

void LcdPages::changeTempRangeString(byte tmin, byte tmax) {
	// MIN Temperature
	char mint[4]{};
	sprintf(mint, "%3d", tmin);
	this->p1_1.at(4) = mint[2];
	this->p1_1.at(5) = mint[3];

	// MAX Temperature
	char maxt[4]{};
	sprintf(maxt, "%3d", tmax);
	this->p1_1.at(12) = maxt[2];
	this->p1_1.at(13) = maxt[3];
}

//----------------------------------------------------------------
// Private LCD updaters
//----------------------------------------------------------------
bool LcdPages::updateLcd() {
	this->_lcd.clear();
	switch (this->actualPage) {
	case 0: // Status page
		lcd.noBlink();
		lcd.setCursor(0, 0);
		lcd.print(this->p0_0);
		lcd.setCursor(0, 1);
		lcd.print(this->p0_1);
		break;
	case 1: // Temperature range view
		lcd.noBlink();
		lcd.setCursor(0, 0);
		lcd.print(this->p1_0);
		lcd.setCursor(0, 1);
		lcd.print(this->p1_1);
		break;
	case 2: // Temperature MIN Edit
		lcd.setCursor(0, 0);
		lcd.print(this->p1_0);
		lcd.setCursor(0, 1);
		lcd.print(this->p1_1);
		lcd.setCursor(5, 1);
		lcd.blink();
	case 3: // Temperature MAX Edit
		lcd.setCursor(0, 0);
		lcd.print(this->p1_0);
		lcd.setCursor(0, 1);
		lcd.print(this->p1_1);
		lcd.setCursor(13, 1);
		lcd.blink();
	}
}


//----------------------------------------------------------------
// Public Value updaters
//----------------------------------------------------------------
void LcdPages::updateTemperatureRange(byte tmin, byte tmax) {
	this->_tmin = tmin;
	this->_tmax = tmax;
	changeTempRangeString();
}

void LcdPages::updateSensorValues(byte t0, byte t1) {
	this->_t0 = t0;
	this->_t1 = t1;
	changeActualTempString();
}

void LcdPages::updateSensorValues(byte t0, byte h0, byte t1, byte h1) {
	this->_h0 = h0;
	this->_h1 = h1;
	updateSensorValues(byte t0, byte t1)
}

void LcdPages::updateFanStatus(byte fanPerc, bool isOff) {
	this->_fanPerc = fanPerc;
	this->_isOff = isOff;
	changeFanString();
}

//----------------------------------------------------------------
// Public Value updaters
//----------------------------------------------------------------
bool LcdPages::buttonPressed(byte btn) {
	bool res = false;
	switch (this->actualPage) {
	case 0: // Status page
		// Only up & down button work
		if (btn == BUTTON_UP || btn == BUTTON_DOWN) {
			this->actualPage = 1;
			res = true;
		}
		else {
			res = false;
		}
		break;
	case 1: // Temperature Range page
		// Up & Down go back to status page
		if (btn == BUTTON_UP || btn == BUTTON_DOWN) {
			this->actualPage = 0;
			res = true;
		}
		// OK to enable change min temperature
		else if (btn == BUTTON_OK) {
			this->_ttemp = this->_tmin; // set temp value
			this->actualPage = 2;
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
			this->actualPage = 3; // go to next value
			res = true;
		}
		if (btn == BUTTON_CANC) {
			changeTempRangeString(this->_tmin, this->_tmax);
			this->actualPage = 1; // exit editing
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
			this->actualPage = 1; // exit editing
			res = true;
		}
		if (btn == BUTTON_CANC) {
			changeTempRangeString(this->_tmin, this->_tmax);
			this->actualPage = 1; // exit editing
			res = true;
		}
		break;
	default:
		res = false;
		break;
	}

	// Update LCD
	return res;
}

