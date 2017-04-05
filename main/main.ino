/////////////////////////////////////////////////////////////////////
// RFID Engine Control
// by a.m.emelianov@gmail.com
//
// Main firmware
//
/////////////////////////////////////////////////////////////////////
#include <Wire.h>             // I2C protocol library
#include <SPI.h>              // SPI protocol library
#include <EEPROM2.h>          // EEPROM helper library
#include <MFRC522.h>          // Library for Mifare RC522 Devices
#include <Adafruit_GFX.h>     // Diaplay helper library
#include <Adafruit_SSD1306.h> // Diaplay library
#include <avr/wdt.h>          // WatchDog Timer library


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


enum CARD { ABSENT, FAILURE, VALID, INVALID };// Card presence state constants

Adafruit_SSD1306  display(OLED_RESET);        // Display class
MFRC522           mfrc522(RF_PIN, RF_RESET);  // RFID reader class
uint32_t          cards[CARD_MAX];            // Array of cards
uint8_t           cardTrys = 0;               // Card read retry counter
bool              start = false;              // Sets when Ignition, resets on Shutdown

// Init hardware and load stored cards
void setup() {
  Serial.begin(115200);
  // Required for Arduino Leonargo board only
  //while (! Serial);                           // Must be disabled for production
  Serial.println("Init display");
  Wire.begin();                               // SSD1306 uses I2C
  display.begin(SSD1306_SWITCHCAPVCC, LCD_ID);// initialize with the I2C addr
  display.clearDisplay();                     // Clear the buffer.
  display.display();                          // Refreash diplay
  pinMode(PIN1,   INPUT_PULLUP);              // Set PIN1 gpio as input
  pinMode(PIN2,   INPUT_PULLUP);              // Set PIN2 gpio as input
  pinMode(RELAY1, OUTPUT);                    // Set RELAY1 gpio as output
  digitalWrite(RELAY1, HIGH);
  pinMode(RELAY2, OUTPUT);                    // Set RELAY2 gpio as output
  digitalWrite(RELAY2, HIGH);
  // Shutdown engine. That is for that case if controller unexpectadly rebooted
  Serial.println("Shutdown engine");
  engineShutdown();                           // Shutdown engine
  ignitionStop();                             // Stop ignition
  Serial.println("Init RFID");
  SPI.begin();                                // RC522 uses SPI
  mfrc522.PCD_Init();                         // Initialize RC522 Hardware
  cardTrys = 0;
  Serial.println("Read stored cards");
  EEPROM_read(EEPROM_CARDS, cards);           // Read stored cards from EEPROM to RAM
  while (cards[0] == 0 || PIN1_HIGH) {        // If first card slot is empty or PIN1 is HIGH waiting for new card to be presented
  //while (PIN1_HIGH) {        // For debug
    Serial.println("Waiting for new card");
    message("Present new card");              // Print message on screen
    while (cardState() != INVALID) { delay(100); } // Waiting until card is prasented
    uint8_t i = 0;
    for (i = 0; i < CARD_MAX; i++) {          // Look for free slot to store card
      if (cards[i] == 0) {                    // if found empty slot
        cards[i] = getCard();                 // copy card ID to slot
        EEPROM_write(EEPROM_CARDS, cards);    // Store cards to EEPROM
        message("Card added");                // Print massage on screen
        break;                                // Stop looking for free slot
      }
    }
    if (i >= CARD_MAX) {                      // No more slots to store cards
      message("Card not added");
      break;                                  // Continue normal startup
    }
    Serial.println("New card added");
    delay(MSG_DELAY);
  }
  wdt_enable(WDTO_8S);                        // Set 8 sec. watchdog timer
}

// Main working cycle
void loop() {
  wdt_reset();                              // Reset watchdog timer
  CARD state = cardState();                 // Try to read card. Possible states are VALID, INVALID, ABSENT, ERROR
  //if (PIN1_HIGH) state = VALID;             // For debug                       
  if (state == VALID) {                     // Valid card is presented
    cardTrys = 0;                           // Reset card read retry counter
    if (! IGNITION) {                       // If Ignition is Off
      ignitionStart();                      // turn it On
    }
    message("Ignition  on");
  } else {                                  // No valid card is presented
    cardTrys++;                             // Increase retry counter
    if (cardTrys > RETRY_MAX) {             // If retry counter exceed max
      if (START) {                          // If engine is not in shutdown state
        engineShutdown();                   // perform shutdown
      }
      if (IGNITION) {                       // If Ignition is on
        ignitionStop();                     // turn it Off
      }
      switch (state) {                        // Print message corresponding to current card presentation state
        case ABSENT:
          message("Waiting   for card..");
          break;
        case FAILURE:
          message("Card read error");
          delay(MSG_DELAY);
          break;
        case INVALID:
          message("Card is   not valid");
          delay(MSG_DELAY);
          break;
      }
    }
  }
  if (START && PIN2_HIGH) {                 // If shutdown button is pressed
    engineShutdown();                       // then shutdown engine
  }
  delay(100);                               // Small delay to not overload peripherals with frequent queries
}

// Get PIIC UID. Using 4-bytes UID.
uint32_t getCard() {
  //Compose 32-bit value from 4 8-bit
  uint32_t curCard = mfrc522.uid.uidByte[0];
  curCard <<= 8;
  curCard = mfrc522.uid.uidByte[1];
  curCard <<= 8;
  curCard = mfrc522.uid.uidByte[2];
  curCard <<= 8;
  curCard = mfrc522.uid.uidByte[3];
  return curCard;
}
// Get card presenrtation state
CARD cardState() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { // If no card presented
    return ABSENT;                          // Return ABSENT
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   // If unable to read card UID
    return FAILURE;                         // return FAILURE
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID card if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println("");
  //mfrc522.PICC_HaltA(); // Stop reading
  uint32_t curCard = getCard();
  for (uint8_t i = 0; i < CARD_MAX; i++) {  // Try to find card presented withng stoted cards
    if (cards[i] == curCard) return VALID;  // Return VALID on success
  }
  return INVALID;                           // return INVALID otherway
}

// Ignition On procedure
void ignitionStart() {
  digitalWrite(RELAY1, LOW);
  start = true;
}

// Ingition Off procedure
void ignitionStop() {
  wdt_reset();                              // Reset watchdog timer before relative long delay
  message("Ignition  off");
  delay(2000);
  digitalWrite(RELAY1, HIGH);
}

// Perform engine shutdown procedure
void engineShutdown() {
  message("Shutting  down");
  digitalWrite(RELAY2, LOW);
  // Divide 6 sec. delay into two 3 sec. delay. I don't like not reset watchdog longer than half of maximum (8 sec.)
  wdt_reset();                            // Reset watchdog
  delay(3000);
  wdt_reset();
  delay(3000);
  digitalWrite(RELAY2, HIGH);
  start = false;
}

// Function to print string on display
void message(char* str) {
  // Operations with buffer does not affect displaying picture until .display() call
  display.clearDisplay();                 // Clear display buffer
  display.setTextSize(2);                 // Text size. 
  display.setTextColor(WHITE);
  display.setCursor(0,0);                 // Start point of text. 0,0 is top left corner
  display.println(str);                   // Print text
  display.display();                      // Show buffer contents
}

