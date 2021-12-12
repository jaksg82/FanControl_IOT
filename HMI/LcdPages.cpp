#include "LcdPages.h"
#include <stdio.h>

//----------------------------------------------------------------
// Constructor and Destructor
//----------------------------------------------------------------
LcdPages::LcdPages() {
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
void LcdPages::createSpecialChars() {
  int char0[] = {0b01110, 0b10001, 0b01110, 0b10001, 0b00100, 0b01010, 0b00000, 0b00100}; // WiFi character
  int char1[] = {0b10001, 0b11011, 0b10101, 0b10001, 0b01100, 0b10010, 0b10010, 0b01101}; // MQTT character
  this->_lcd->createChar(0, char0);
  this->_lcd->createChar(1, char1);
}


//----------------------------------------------------------------
// Private String updaters for LCD 1602
//----------------------------------------------------------------
#ifdef LCD_SCREEN_1602A
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
  //Check what page is active
  if (this->_actualPage == 0) { updateLcd(); }
}

void LcdPages::changeTimeString() {
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
  changeTempRangeString(this->_tmin, this->_tmax, 0);
}

void LcdPages::changeTempRangeString(byte tmin, byte tmax, byte curPos) {
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

  // Cursor position
  if(curPos == 1) {
    // Editing T MIN
    this->p1_1[3] = '>';
    this->p1_1[3] = ':';
  } else if(curPos == 2) {
    // Editing T MAX
    this->p1_1[3] = ':';
    this->p1_1[3] = '>';
  } else {
    // Visualization mode
    this->p1_1[3] = ':';
    this->p1_1[3] = ':';
  }

  updateLcd();
}

//----------------------------------------------------------------
// Private String updaters for LCD 2004
//----------------------------------------------------------------
#else
void LcdPages::degreeFix() {
  this->p0_1[6] = 0xDF;
  this->p0_1[7] = 'C';
  this->p0_1[17] = 0xDF;
  this->p0_1[18] = 'C';
}

void LcdPages::changeActualTempString() {
  // Temperature Sensor 0
  char t0c[5]{};
  sprintf(t0c, "%4d", this->_t0);
  //char t0c0 = t0c[0];
  char t0c1 = t0c[1];
  char t0c2 = t0c[2];
  char t0c3 = t0c[3];
  this->p0_1[3] = t0c1;
  this->p0_1[4] = t0c2;
  this->p0_1[5] = t0c3;

  // Temperature Sensor 1
  char t1c[5]{};
  sprintf(t1c, "%4d", this->_t1);
  //char t1c0 = t1c[0];
  char t1c1 = t1c[1];
  char t1c2 = t1c[2];
  char t1c3 = t1c[3];
  this->p0_1[14] = t1c1;
  this->p0_1[15] = t1c2;
  this->p0_1[16] = t1c3;

  //Check what page is active
  if (this->_actualPage == 0) { updateLcd(); }
}

void LcdPages::changeFanString() {
  // Fan Duty Percentage
  if (_isOn) {
    char fanc[5]{};
    sprintf(fanc, "%4d", this->_fanPerc);
    //char c0 = fanc[0];
    char c1 = fanc[1];
    char c2 = fanc[2];
    char c3 = fanc[3];
    this->p0_2[5] = c1;
    this->p0_2[6] = c2;
    this->p0_2[7] = c3;
    this->p0_2[8] = '%';
  } else {
    this->p0_2[5] = ' ';
    this->p0_2[6] = 'O';
    this->p0_2[7] = 'F';
    this->p0_2[8] = 'F';
  }

  //Check what page is active
  if (this->_actualPage == 0) { updateLcd(); }
}

void LcdPages::changeTimeString() {
  // Time Stamp
  char yyc[6]{};
  char mtc[4]{};
  char ddc[4]{};
  char hhc[4]{};
  char mmc[4]{};
  char ssc[4]{};
  sprintf(yyc, "%04d", this->_yy);
  sprintf(mtc, "%02d", this->_mt);
  sprintf(ddc, "%02d", this->_dd);
  sprintf(hhc, "%02d", this->_hh);
  sprintf(mmc, "%02d", this->_mm);
  sprintf(ssc, "%02d", this->_ss);
  this->p0_0[0] = (char)yyc[0];
  this->p0_0[1] = (char)yyc[1];
  this->p0_0[2] = (char)yyc[2];
  this->p0_0[3] = (char)yyc[3];
  this->p0_0[4] = '-';
  this->p0_0[5] = (char)mtc[0];
  this->p0_0[6] = (char)mtc[1];
  this->p0_0[7] = '-';
  this->p0_0[8] = (char)ddc[0];
  this->p0_0[9] = (char)ddc[1];
  this->p0_0[10] = ' ';
  this->p0_0[11] = (char)hhc[0];
  this->p0_0[12] = (char)hhc[1];
  this->p0_0[13] = ':';
  this->p0_0[14] = (char)mmc[0];
  this->p0_0[15] = (char)mmc[1];
  this->p0_0[16] = ':';
  this->p0_0[17] = (char)ssc[0];
  this->p0_0[18] = (char)ssc[1];
  this->p0_0[19] = ' ';

  //Check what page is active
  if (this->_actualPage == 0) { updateLcd(); }
}

void LcdPages::changeTempRangeString() {
  changeTempRangeString(this->_tmin, this->_tmax, (byte)0);
}

void LcdPages::changeTempRangeString(byte tmin, byte tmax, byte curPos) {
  // MIN Temperature
  char mint[5]{};
  sprintf(mint, "%4d", tmin);
  //char c0 = mint[0];
  //char c1 = mint[1];
  char c2 = mint[2];
  char c3 = mint[3];
  this->p0_3[7] = c2;
  this->p0_3[8] = c3;

  // MAX Temperature
  char maxt[5]{};
  sprintf(maxt, "%4d", tmax);
  //char c0x = maxt[0];
  //char c1x = maxt[1];
  char c2x = maxt[2];
  char c3x = maxt[3];
  this->p0_3[17] = c2x;
  this->p0_3[18] = c3x;

  // Cursor position
  if(curPos == 1) {
    // Editing T MIN
    this->p0_3[6] = '>';
    this->p0_3[9] = '<';
    this->p0_3[10] = ' ';
    this->p0_3[11] = ' ';
    this->p0_3[12] = ' ';
    this->p0_3[13] = ' ';
    this->p0_3[14] = ' ';
    this->p0_3[15] = ' ';
    this->p0_3[16] = ' ';
    this->p0_3[19] = ' ';
  } else if(curPos == 2) {
    // Editing T MAX
    this->p0_3[6] = ' ';
    this->p0_3[9] = ' ';
    this->p0_3[10] = ' ';
    this->p0_3[11] = ' ';
    this->p0_3[12] = ' ';
    this->p0_3[13] = ' ';
    this->p0_3[14] = ' ';
    this->p0_3[15] = ' ';
    this->p0_3[16] = '>';
    this->p0_3[19] = '<';
  } else {
    // Visualization mode
    this->p0_3[6] = ' ';
    this->p0_3[9] = ' ';
    this->p0_3[10] = '<';
    this->p0_3[11] = '=';
    this->p0_3[12] = '=';
    this->p0_3[13] = '=';
    this->p0_3[14] = '=';
    this->p0_3[15] = '>';
    this->p0_3[16] = ' ';
    this->p0_3[19] = ' ';
  }
  updateLcd();
}


#endif
//----------------------------------------------------------------
// Private LCD updaters
//----------------------------------------------------------------
#ifdef LCD_SCREEN_1602A
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
        break;
      case 3: // Temperature MAX Edit
        this->_lcd->setCursor(0, 0);
        this->_lcd->print(this->p1_0);
        this->_lcd->setCursor(0, 1);
        this->_lcd->print(this->p1_1);
        this->_lcd->setCursor(13, 1);
        this->_lcd->blink();
        break;
    }
    updateIotChars();
    return true;
  } else {
    return false;
  }
}

#else  // LCD 2004A
bool LcdPages::updateLcd() {
  if (isAttached) {
    this->_lcd->clear();
    this->_lcd->setCursor(0, 0);
    this->_lcd->print(this->p0_0);
    this->_lcd->setCursor(0, 1);
    this->_lcd->print(this->p0_1);
    this->_lcd->setCursor(0, 2);
    this->_lcd->print(this->p0_2);
    this->_lcd->setCursor(0, 3);
    this->_lcd->print(this->p0_3);

    switch (this->_actualPage) {
      case 0: // Status page
      case 1: // Not used for Lcd 2004
        this->_lcd->noBlink();
        break;
      case 2: // Temperature MIN Edit
        this->_lcd->setCursor(8, 3);
        this->_lcd->blink();
        break;
      case 3: // Temperature MAX Edit
        this->_lcd->setCursor(18, 3);
        this->_lcd->blink();
        break;
    }
    updateIotChars();
    return true;
  } else {
    return false;
  }
}


#endif

void LcdPages::updateIotChars() {
  byte wCol, wRow, mCol, mRow;
  if(_isLCD1602) {
    wCol = 14;
    wRow = 1;
    mCol = 15;
    mRow = 1;
  } else {
    wCol = 14;
    wRow = 2;
    mCol = 16;
    mRow = 2;
  }
  if (this->_actualPage == 0) {
    this->_lcd->setCursor(wCol, wRow);
    if (_isWifi) {
      this->_lcd->write(byte(0));
    } else {
      this->_lcd->write('-');
    }
    this->_lcd->setCursor(mCol, mRow);
    if (_isMqtt) {
      this->_lcd->write(byte(1));
    } else {
      this->_lcd->write('-');
    }
  }
}


//----------------------------------------------------------------
// Public Value updaters
//----------------------------------------------------------------
void LcdPages::updateTemperatureRange(byte tmin, byte tmax) {
  if (this->_tmin != tmin || this->_tmax != tmax){
    this->_tmin = tmin;
    this->_tmax = tmax;
    if(this->_actualPage <= 1) {
      this->changeTempRangeString();
    }
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
    this->changeTimeString();
  }
}

void LcdPages::updateTimeStamp(uint16_t yy, byte mt, byte dd, byte hh, byte mm, byte ss) {
  this->_yy = yy;
  this->_mt = mt;
  this->_dd = dd;
  this->_hh = hh;
  this->_mm = mm;
  this->_ss = ss;
  this->changeTimeString();
}

void LcdPages::updateIotStatus(bool isWifiConnected, bool isMqttConnected) {
  _isWifi = isWifiConnected;
  if (_isWifi) {
    _isMqtt = isMqttConnected;
  } else {
    _isMqtt = false; // MQQT can NOT be connected if the WiFi are not available
  }

  updateIotChars();
}
//----------------------------------------------------------------
// Button interaction handling
//----------------------------------------------------------------
//
//  TODO: Rewrite the code 
//  TODO: Implement the ifdef switch between lcd1602 and lcd2004
//
ByteRange LcdPages::buttonPressed(byte btn) {
  bool res = false;
  ByteRange resRange = ByteRange(0,255);
  
  switch(this->_actualPage) {
    case 0: // Status page
      // Only up & down button work
      if(btn == BUTTON_UP || btn == BUTTON_DOWN) {
        if(_isLCD1602) {
          this->_actualPage = 1;
          res = true;
        } else {
          res = false;
        }
      } else if(btn == BUTTON_OK) {
        if(_isLCD1602) {
          res = false;
        } else {
          this->_actualPage = 2;
          this->modTemp.SetMinMax(this->_tmin, this->_tmax); // Update temp range
          changeTempRangeString(this->modTemp.GetMin(), this->modTemp.GetMax(), 1);
          res = true;
        }
      } else {
        res = false;
      }
      break;
    case 1: // Temperature Range page
      if(_isLCD1602) {
        // Up & Down go back to status page
        if(btn == BUTTON_UP || btn == BUTTON_DOWN) {
          this->_actualPage = 0;
          res = true;
        } else if(btn == BUTTON_OK) {  // OK to enable change min temperature
          this->modTemp.SetMinMax(this->_tmin, this->_tmax); // Update temp range
          this->_actualPage = 2;
          res = true;
        } else {  // Canc button do nothing
          res = false;
        }
      } else { // Page 1 NOT exist on LCD2004 
        res = false;
      }
      break;
    case 2: // Min temperature change page
      if(btn == BUTTON_UP) {
        if(this->modTemp.IncreaseMin()) {
          changeTempRangeString(this->modTemp.GetMin(), this->modTemp.GetMax(), 1);
          res = true;
        } else {
          res = false;
        }
      }
      if(btn == BUTTON_DOWN) {
        if(this->modTemp.DecreaseMin()) {
          changeTempRangeString(this->modTemp.GetMin(), this->modTemp.GetMax(), 1);
          res = true;
        } else {
          res = false;
        }
      }
      if(btn == BUTTON_OK) { // Move cursor to change the MAX value
        changeTempRangeString(this->modTemp.GetMin(), this->modTemp.GetMax(), 2);
        this->_actualPage = 3; // go to next value
        res = true;
      }
      if(btn == BUTTON_CANC) { // Discard the changes
        changeTempRangeString(this->_tmin, this->_tmax, 0);
        this->_actualPage = _isLCD1602 ? 1 : 0; // exit editing
        res = true;
      }
      break;
    case 3: // Max temperature change page
      if(btn == BUTTON_UP) {
        if(this->modTemp.IncreaseMax()) {
          changeTempRangeString(this->modTemp.GetMin(), this->modTemp.GetMax(), 2);
          res = true;
        } else {
          res = false;
        }
      }
      if(btn == BUTTON_DOWN) {
        if(this->modTemp.DecreaseMax()) {
          changeTempRangeString(this->modTemp.GetMin(), this->modTemp.GetMax(), 2);
          res = true;
        } else {
          res = false;
        }
      }
      if(btn == BUTTON_OK) {  // ACCEPT the changes and send to PLC
        changeTempRangeString(this->modTemp.GetMin(), this->modTemp.GetMax(), 0);
        this->_actualPage = _isLCD1602 ? 1 : 0; // exit editing
        res = true;
        resRange = this->modTemp; // Return the new temperature range
      }
      if(btn == BUTTON_CANC) {
        changeTempRangeString(this->_tmin, this->_tmax, 0);
        this->_actualPage = _isLCD1602 ? 1 : 0; // exit editing
        res = true;
      }
      break;
    default:
      res = false;
      break;
    }

    // Update LCD
    if (res) { updateLcd(); }
    return resRange;
}
