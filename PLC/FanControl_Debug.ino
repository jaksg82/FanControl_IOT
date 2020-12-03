// Debug macro to print messages to serial
void DEBUG(int x) {
  if (Serial) {
    Serial.print (x);
  }
}
void DEBUG(String x) {
  if (Serial) {
    Serial.print (x);
  }
}
void DEBUG(byte x) {
  if (Serial) {
    Serial.print (x);
  }
}
void DEBUG(double x) {
  if (Serial) {
    Serial.print (x);
  }
}
void DEBUG(float x) {
  if (Serial) {
    Serial.print (x);
  }
}
void DEBUG(long x) {
  if (Serial) {
    Serial.print (x);
  }
}

bool checkUsb() {
  if (USBSTA & (1 << VBUS)) { //checks state of VBUS
    if (!Serial) Serial.begin(115200);
    while (!Serial) {} // wait for serial port to connect. Needed for native USB
    lcd.setCursor(15, 1);
    lcd.write(1);
  }
  else {
    if (Serial) Serial.end();
    lcd.setCursor(15, 1);
    lcd.write(" ");
  }
}
