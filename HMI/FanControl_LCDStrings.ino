/*
// Create a fixed length string for the output to LCD and serial
void FormatDhtString() {
  // Temperature line
  String t1 = "";
  String t2 = "";
  String h1 = "";
  String h2 = "";

  if (Temperature1 < 9) { t1 = " " + (int)Temperature1; }
  else { t1 = (int)Temperature1; }
  if (Temperature2 < 9) { t2 = " " + (int)Temperature2; }
  else { t2 = (int)Temperature2; }
  if (Humidity1 < 9) { h1 = " " + (int)Humidity1; }
  else { h1 = (int)Humidity1; }
  if (Humidity2 < 9) { h2 = " " + (int)Humidity2; }
  else { h2 = (int)Humidity2; }
  
  DebugDHT = "T1: " + t1 + "C H1: " + h1 + "% T2: " + t2 + "C H2: " + h2 + "%";
  LcdDHT = "T1: " + t1 + "C T2: " + t2 + "C ";

  LcdNeedUpdate = true;
}

void FormatStatusString() {
  //bool fanRunning
  String percS = "";
  int perc = (duty / 255) * 100;
  String strPerc = String(perc);

  switch (perc) {
    case 0:
      percS = "  0";
      break;
    case 1 ... 9:
      percS = "  " + strPerc;
      break;
    case 10 ... 99:
      percS = " " + strPerc;
      break;
    case 100:
      percS = "100";
      break;
    default:
      percS = "###";
      break;
  }
 
  LcdStatus = "Fan: " + percS + "%  " + (fanRelay.isRelayOn() ? " ON" : "OFF");
  
  LcdNeedUpdate = true;
}

void FormatTargetString(bool IsViewMode, bool IsTargetMin) {
  LcdTarget = "Temperature: ";
  if (IsViewMode) {
    if (IsTargetMin) {
      LcdTarget += String(TargetTempMin);
    }
    else { LcdTarget += String(TargetTempMax); }
  }
  else { LcdTarget += String(TempMod); }
}


// Write the proper strings to the LCD
void updateLCD() {
  lcd.clear();
  switch (actualPage) {
    // Status Page
    case 1:
      lcd.noBlink();
      lcd.setCursor(0, 0);
      lcd.print(LcdDHT);
      lcd.setCursor(0, 1);
      lcd.print(LcdStatus);
      lcd.setCursor(6, 0);    //Substitute C with degCelsius
      lcd.write(0);
      lcd.setCursor(14, 0);
      lcd.write(0);
      break;
    // VIEW Target 1  
    case 2:
      FormatTargetString(true, true);
      lcd.noBlink();
      lcd.setCursor(0, 0);
      lcd.print(LcdTargetMinTitle);
      lcd.setCursor(0, 1);
      lcd.print(LcdTarget);
      break;
    // VIEW Target 2  
    case 3:
      FormatTargetString(true, false);
      lcd.noBlink();
      lcd.setCursor(0, 0);
      lcd.print(LcdTargetMaxTitle);
      lcd.setCursor(0, 1);
      lcd.print(LcdTarget);
      break;
    // CHANGE Target 1  
    case 4:
      FormatTargetString(false, true);
      lcd.setCursor(0, 0);
      lcd.print(LcdTargetMinTitle);
      lcd.setCursor(0, 1);
      lcd.print(LcdTarget);
      lcd.setCursor(15, 1);
      lcd.blink();
      break;
    // CHANGE Target 2  
    case 5:
      FormatTargetString(false, false);
      lcd.setCursor(0, 0);
      lcd.print(LcdTargetMaxTitle);
      lcd.setCursor(0, 1);
      lcd.print(LcdTarget);
      lcd.setCursor(15, 1);
      lcd.blink();
      break;
  }

  LcdNeedUpdate = false;
}

// Called when a button is pressed
void buttonClick(int btnPressed)
{
  switch (actualPage) {
    // Status Page
    case 1:
      if (btnPressed == 1) { actualPage = 3; }
      else if (btnPressed == 2) { actualPage = 2; }
      break;
    // View Target Temp 1 Page
    case 2:
      if (btnPressed == 1) { actualPage = 1; }
      else if (btnPressed == 2) { actualPage = 3; }
      else if (btnPressed == 3) { actualPage = 4; TempMod = TargetTempMin; }
      break;
    // Viev Target Temp 2 Page
    case 3:  
      if (btnPressed == 1) { actualPage = 2; }
      else if (btnPressed == 2) { actualPage = 1; }
      else if (btnPressed == 3) { actualPage = 5; TempMod = TargetTempMax; }
      break;
    // Change Target Temp 1 Page
    case 4:
      if (btnPressed == 1) { TempMod += 1; FormatTargetString(false, true); }
      else if (btnPressed == 2) { TempMod -= 1; FormatTargetString(false, true); }
      else if (btnPressed == 3) { actualPage = 2; UpdateTargets(TempMod, true); FormatTargetString(true, true); } // Save the new target
      else if (btnPressed == 4) { actualPage = 2; FormatTargetString(true, true); } // Discard the new value
      break;
    // Change Target Temp 2 Page
    case 5:
      if (btnPressed == 1) { TempMod += 1; FormatTargetString(false, false); }
      else if (btnPressed == 2) { TempMod -= 1; FormatTargetString(false, false); }
      else if (btnPressed == 3) { actualPage = 3; UpdateTargets(TempMod, false); FormatTargetString(true, false); } // Save the new target
      else if (btnPressed == 4) { actualPage = 3; FormatTargetString(true, false); } // Discard the new value
      break;
  }
  LcdNeedUpdate = true;
  
  DEBUG("actualPage: ");
  DEBUG(actualPage);
  DEBUG(" btnPressed: ");
  DEBUG(btnPressed);
  DEBUG(" T1: ");
  DEBUG(TargetTempMin);
  DEBUG(" T2: ");
  DEBUG(TargetTempMax);
  DEBUG(" TM: ");
  DEBUG(TempMod);
} */
