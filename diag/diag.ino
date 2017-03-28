/////////////////////////////////////////////////////////////////////
// RFID Engine Control
// by a.m.emelianov@gmail.com
//
// Diagnostic firmware
//
/////////////////////////////////////////////////////////////////////

#include <Wire.h>

#include <EEPROM2.h>
#define EEPROM_CLEAR 512

// Shutdown pin. Shuold be connected to ground to initiate action.
#define PIN2          A2
// Card program pin. Should be connected to ground to initiate action
#define PIN1          A3
// RFID reader SPI SS pin
#define RF_PIN        10
// SPI MOSI           11
// SPI MISO           12
// SPI SCK            13
// RFID reset pin
#define RF_RESET      11
// Realy 1 pin
#define RELAY1        A0
// Relay 2 pin
#define RELAY2        A1
// OLED display I2C ID. Scan I2C bus to detect what ID is set at Your display
#define LCD_ID        0x3C
// Maximum number of stored card slots
#define CARD_MAX      5
// Ignition macro. Just to make code clean.
#define IGNITION      (digitalRead(RELAY1)==LOW)
// Start macro
#define START         start
// Pin 1 state check macro
#define PIN1_HIGH     (digitalRead(PIN1)==LOW)
// Pin 2 state check macro
#define PIN2_HIGH     (digitalRead(PIN2)==LOW)
// Count fo retries to read card
#define RETRY_MAX      5
// Card storage EEPROM position
#define EEPROM_CARDS  0
// Display reset pin (not really used)
#define OLED_RESET    4
// Delay for message (2 sec.)
#define MSG_DELAY     2000

void setup() {
  Serial.begin(115200);
  //while (!Serial);             // Leonardo: wait for serial monitor
#ifdef EEPROM_CLEAR
 for (uint16_t i = 0; i < EEPROM_CLEAR; i++) {
  EEPROM_write_byte(i, 0);
 }
#endif
 pinMode(PIN1, INPUT_PULLUP);
 pinMode(PIN2, INPUT_PULLUP);
 pinMode(RELAY1, OUTPUT);
 digitalWrite(RELAY1, HIGH);
 pinMode(RELAY2, OUTPUT);
 digitalWrite(RELAY2, HIGH);
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
  digitalWrite(RELAY1, LOW);
  delay(2000);
  Serial.println("Realay 2 ON");
  digitalWrite(RELAY2, LOW);
  delay(2000);
  Serial.println("Realay 1 OFF");
  digitalWrite(RELAY1, HIGH);
  delay(2000);
  Serial.println("Realay 2 OFF");
  digitalWrite(RELAY2, HIGH);
  delay(2000);

  delay(5000);           // wait 5 seconds for next scan
}
