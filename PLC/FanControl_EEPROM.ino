void GetConfigFromEeprom(byte* tmin, byte* tmax) {
  byte vmin = EEPROM.read(22);
  byte vmax = EEPROM.read(33);
  *tmin = vmin; //> 0 ? vmin : 25; //Set default value if the eeprom value is zero
  *tmax = vmax; //> 0 ? vmax : 35; //Set default value if the eeprom value is zero
}

void SetConfigToEeprom(byte* tmin, byte* tmax) {
  byte vmin = EEPROM.read(22);
  byte vmax = EEPROM.read(33);
  if (vmin != *tmin) { EEPROM.write(22, *tmin); }
  if (vmax != *tmax) { EEPROM.write(33, *tmax); }

}

byte FitInTemp(byte value) {
  if (value < 10) {
    return 10;
  }
  else {
    if (value > 60) {
      return 60;
    }
    else {
      return value;
    }
  }
}
