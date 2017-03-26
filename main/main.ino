/////////////////////////////////////////////////////////////////////
// RFID Engine Control
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

#define PIN2          A5
#define RF_PIN        10
#define RF_RESET      11
#define RELAY1        A0
#define RELAY2        A1
#define LCD_ID        0x3C
#define CARD_MAX      5
#define IGNITION      (digitalRead(RELAY1)==LOW)
#define START         start
#define PIN1_HIGH     (digitalRead(PIN1)==LOW)
#define PIN2_HIGH     (digitalRead(PIN2)==LOW)
#define TRYS_MAX      5
#define EEPROM_CARDS  0
#define OLED_RESET    4
#define PIN1          A4

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
  while (! Serial);                           // Must be disabled for production
  Serial.println("Init display");
  Wire.begin();                               // SSD1306 uses I2C
  display.begin(SSD1306_SWITCHCAPVCC, LCD_ID);// initialize with the I2C addr
  display.clearDisplay();                     // Clear the buffer.
  display.display();                          // Refreash diplay
  pinMode(PIN1,   INPUT_PULLUP);                     // Set PIN1 gpio as input
  pinMode(PIN2,   INPUT_PULLUP);                     // Set PIN2 gpio as input
  pinMode(RELAY1, OUTPUT);                    // Set RELAY1 gpio as output
  digitalWrite(RELAY1, HIGH);
  pinMode(RELAY2, OUTPUT);                    // Set RELAY2 gpio as output
  digitalWrite(RELAY2, HIGH);
  // Shutdown engine. That is for that case if controller unexpactadly rebooted
  Serial.println("Shutdown engine");
  engineShutdown();                           // Shutdown engine
  ignitionStop();                             // Stop ignition
  Serial.println("Init RFID");
  SPI.begin();                                // RC522 uses SPI
  mfrc522.PCD_Init();                         // Initialize RC522 Hardware
  cardTrys = 0;
  Serial.println("Read stored cards");
  EEPROM_read(EEPROM_CARDS, cards);           // Read stored cards from EEPROM to RAM
  //while (cards[0] == 0 || PIN1_HIGH) {        // If first card slot is empty or PIN1 is HIGH waiting for new card to be presented
  while (PIN1_HIGH) {        // If first card slot is empty or PIN1 is HIGH waiting for new card to be presented
    Serial.println("Waiting for new card");
    message("Present new card");
    while (getCard() != INVALID) { delay(100); }
    uint8_t i = 0;
    for (i = 0; i < CARD_MAX; i++) {          // Look for free slot to store card
      if (cards[i] == 0) {                    // if found empty slot
        cards[i] = getCard();                 // copy card ID to slot
        EEPROM_write(EEPROM_CARDS, cards);    // Store cards to EEPROM
        message("Card added");
      }
    }
    if (i >= CARD_MAX) {                      // No more slots to store cards
      message("Card not added");
      break;                                  // Continue normal startup
    }
    Serial.println("New card added");
  }
  wdt_enable(WDTO_8S);                        // Set 8 sec. watchdog timer
}

// Main working cicle
void loop() {
  wdt_reset();                  // Reset watchdog timer
  CARD state = cardState();
  if (PIN1_HIGH) {
    state = VALID;
  }
  if (state == VALID) {
    cardTrys = 0;
    if (! IGNITION) {
      ignitionStart();
    }
    message("Ignition  on");
  } else {
    cardTrys++;
    if (cardTrys > TRYS_MAX) {
      if (START) {
        engineShutdown();
      }
      if (IGNITION) {
        ignitionStop();
      }
    switch (state) {
      case ABSENT:
        message("Waiting   for card...");
        break;
      case FAILURE:
        message("Card read error");
        break;
      case INVALID:
        message("Card is not valid");
        break;
    }
    }
  }
  if (START && PIN2_HIGH) {
    engineShutdown();
  }
  delay(100);
}

uint32_t getCard() {
  uint32_t curCard = mfrc522.uid.uidByte[0];
  curCard <<= 8;
  curCard = mfrc522.uid.uidByte[1];
  curCard <<= 8;
  curCard = mfrc522.uid.uidByte[2];
  curCard <<= 8;
  curCard = mfrc522.uid.uidByte[3];
  return curCard;
}
CARD cardState() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return ABSENT;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return FAILURE;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println("");
  //mfrc522.PICC_HaltA(); // Stop reading
  uint32_t curCard = mfrc522.uid.uidByte[0];
  curCard <<= 8;
  curCard = mfrc522.uid.uidByte[1];
  curCard <<= 8;
  curCard = mfrc522.uid.uidByte[2];
  curCard <<= 8;
  curCard = mfrc522.uid.uidByte[3];
  for (uint8_t i = 0; i < CARD_MAX; i++) {
    if (cards[i] == curCard) return VALID;
  }
  return INVALID;
}

void ignitionStart() {
  digitalWrite(RELAY1, LOW);
  start = true;
}
void ignitionStop() {
  wdt_reset();
  message("Ignition  off");
  delay(2000);
  digitalWrite(RELAY1, HIGH);
}
void engineShutdown() {
  message("Sutting   down");
  digitalWrite(RELAY2, LOW);
  wdt_reset();
  delay(3000);
  wdt_reset();
  delay(3000);
  digitalWrite(RELAY2, HIGH);
  start = false;
}
void message(char* str) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(str);
  display.display();
}

