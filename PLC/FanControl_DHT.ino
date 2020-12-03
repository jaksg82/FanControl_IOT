int GetSensorData(SimpleDHT11* pSensor, byte* ptemperature, byte* phumidity) {
    byte t = 0, h = 0;
    byte pdata[40] = {0};
    if (!pSensor->read(&t, &h, pdata))
    {
      DEBUG("Temperature: ");
      DEBUG(t);
      DEBUG("\n");

      if (!isnan(t)) { *ptemperature = t; }
      if (!isnan(h)) { *phumidity = h; }
      return 1;
    }
    else
    {
      // If there's an error in the sensor, wait 5 seconds to let the communication reset
      DEBUG("Temperature: Error\n");
      return 0;
    }

}

void UpdateTargets(byte newTemp, bool IsMin) {
  if (IsMin) {
    TargetTempMin = newTemp;
    if (TargetTempMax <= newTemp) {
      TargetTempMax = newTemp + 1;
    }
  }
  else {
    TargetTempMax = newTemp;
    if (TargetTempMin >= newTemp) {
      TargetTempMin = newTemp - 1;
    }
  }
  SetConfigToEeprom(&TargetTempMin, &TargetTempMax);
}

bool isTargetValid() {
  if (TargetTempMin >= 10 && TargetTempMin <= 59) {
    if (TargetTempMax >= 11 && TargetTempMax <= 60) {
      if (TargetTempMin < TargetTempMax) {
        return true;
      }
    }
  }
  return false;
}
