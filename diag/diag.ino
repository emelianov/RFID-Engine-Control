// RFID Engine Control
//
// Clear EEPROM & Check Peripherals
//

#include <Wire.h>

#include <EEPROM2.h>
#define EEPROM_CLEAR 512

#define PIN2      2
#define RELAY1    3
#define RELAY2    4

void setup() {
  Serial.begin(115200);
  while (!Serial);             // Leonardo: wait for serial monitor
#ifdef EEPROM_CLEAR
 for (uint16_t i = 0; i < EEPROM_CLEAR; i++) {
  EEPROM_write_byte(i, 0);
 }
#endif
 pinMode(PIN2, INPUT);
 pinMode(RELAY1, OUTPUT);
 digitalWrite(RELAY1, LOW);
 pinMode(RELAY2, OUTPUT);
 digitalWrite(RELAY2, LOW);
 Wire.begin();
 Serial.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int n;

  Serial.println("Looking for I2C devices...");

  n = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.print("Found ar address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
      n++;
    }
  }
  if (n == 0) {
    Serial.println("Found nothing\n");
  } else {
    Serial.println("done\n");
  }
  Serial.println("Realay 1 ON");
  digitalWrite(RELAY1, HIGH);
  delay(1000);
  Serial.println("Realay 2 ON");
  digitalWrite(RELAY2, HIGH);
  delay(1000);
  Serial.println("Realay 1 OFF");
  digitalWrite(RELAY1, LOW);
  delay(1000);
  Serial.println("Realay 2 OFF");
  digitalWrite(RELAY1, LOW);
  delay(1000);

  delay(5000);           // wait 5 seconds for next scan
}
